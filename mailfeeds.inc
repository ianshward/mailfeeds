<?php

// TODO: refactor mailhandler.inc and use it instead

/**
 * Establish a connection to the mailbox specified by the array $mailbox
 */
function mailfeeds_open_mailbox($mailbox) {
  if ($mailbox['domain']) {
    if ($mailbox['imap'] == 1) {
      $box = '{'. $mailbox['domain'] .':'. $mailbox['port'] . $mailbox['extraimap'] .'}'. $mailbox['folder'];
    }
    else {
      $box = '{'. $mailbox['domain'] .':'. $mailbox['port'] .'/pop3'. $mailbox['extraimap'] .'}'. $mailbox['folder'];
    }
    $result = imap_open($box, $mailbox['name'], $mailbox['pass']);
    $err = 'domain';
  }
  else {
    $box = $mailbox['folder'];
    $result = imap_open($box, '', '');
  }
  return $result;
}

function mailfeeds_get_unread_messages($result) {
  $unread_messages = array();
  $number_of_messages = imap_num_msg($result);
  for ($i = 1; $i <= $number_of_messages; $i++) {
    $header = imap_header($result, $i);
    // only process new messages
    if ($header->Unseen != 'U' && $header->Recent != 'N') {
      continue;
    }
    $unread_messages[] = imap_uid($result, $i);
  }
  return $unread_messages;
}

function mailfeeds_retrieve_message(&$result, $i, $mailbox) {
  $item = array();
  $header = imap_header($result, imap_msgno($result, $i));
  // Initialize the subject in case it's missing.
  if (!isset($header->subject)) {
    $header->subject = '';
  }
  $mime = explode(',', $mailbox['mime']);
  // Get the first text part - this will be the node body
  $origbody = mailfeeds_get_part($result, $i, $mime[0]);
  // If we didn't get a body from our first attempt, try the alternate format (HTML or PLAIN)
  if (!$origbody) {
    $origbody = mailfeeds_get_part($result, $i, $mime[1]);
  }
  // Parse MIME parts, so all mailhandler modules have access to
  // the full array of mime parts without having to process the email.
  $mimeparts = mailfeeds_get_parts($result, $i);

  $item['header'] = $header;
  $item['mime'] = $mime;
  $item['origbody'] = $origbody;
  $item['mimeparts'] = $mimeparts;
  return $item;
}

/**
 * Returns the first part with the specified mime_type
 *
 * USAGE EXAMPLES - from php manual: imap_fetch_structure() comments
 * $data = get_part($stream, $msg_number, "TEXT/PLAIN"); // get plain text
 * $data = get_part($stream, $msg_number, "TEXT/HTML"); // get HTML text
 */
function mailfeeds_get_part($stream, $msg_number, $mime_type, $structure = false, $part_number = false) {

  if (!$structure) {
    $structure = imap_fetchstructure($stream, $msg_number, FT_UID);
  }
  if ($structure) {
    $encoding = variable_get('mailhandler_default_encoding', 'UTF-8');
    foreach ($structure->parameters as $parameter) {
      if (strtoupper($parameter->attribute) == 'CHARSET') {
        $encoding = $parameter->value;
      }
    }
    if ($mime_type == mailfeeds_get_mime_type($structure)) {
      if (!$part_number) {
        $part_number = '1';
      }
      $text = imap_fetchbody($stream, $msg_number, $part_number, FT_UID);
      if ($structure->encoding == ENCBASE64) {
        return drupal_convert_to_utf8(imap_base64($text), $encoding);
      }
      else if ($structure->encoding == ENCQUOTEDPRINTABLE) {
        return drupal_convert_to_utf8(quoted_printable_decode($text), $encoding);
      }
      else {
        return drupal_convert_to_utf8($text, $encoding);
      }
    }
    if ($structure->type == TYPEMULTIPART) { /* multipart */
      $prefix = '';
      while (list($index, $sub_structure) = each ($structure->parts)) {
        if ($part_number) {
          $prefix = $part_number .'.';
        }
        $data = mailfeeds_get_part($stream, $msg_number, $mime_type, $sub_structure, $prefix . ($index + 1));
        if ($data) {
          return $data;
        }
      }
    }
  }

  return false;
}


/**
 * Returns an array of parts as file objects
 *
 * @param
 * @param $structure
 *   A message structure, usually used to recurse into specific parts
 * @param $max_depth
 *   Maximum Depth to recurse into parts.
 * @param $depth
 *   The current recursion depth.
 * @param $part_number
 *   A message part number to track position in a message during recursion.
 * @return
 *   An array of file objects.
 */
function mailfeeds_get_parts($stream, $msg_number, $max_depth = 10, $depth = 0, $structure = FALSE, $part_number = FALSE) {
  $parts = array();

  // Load Structure.
  if (!$structure && !$structure = imap_fetchstructure($stream, $msg_number, FT_UID)) {
    watchdog('mailhandler', 'Could not fetch structure for message number %msg_number', array('%msg_number' => $msg_number), WATCHDOG_NOTICE);
    return $parts;
  }

  // Recurse into multipart messages.
  if ($structure->type == TYPEMULTIPART) {
    // Restrict recursion depth.
    if ($depth >= $max_depth) {
      watchdog('mailhandler', 'Maximum recursion depths met in mailhander_get_structure_part for message number %msg_number.',  array('%msg_number' => $msg_number), WATCHDOG_NOTICE);
      return $parts;
    }
    $prefix = '';
    foreach($structure->parts as $index => $sub_structure) {
      // If a part number was passed in and we are a multitype message, prefix the
      // the part number for the recursive call to match the imap4 dot seperated part indexing.
      if ($part_number) {
        $prefix = $part_number .'.';
      }
      $sub_parts =  mailfeeds_get_parts($stream, $msg_number, $max_depth, $depth + 1,
        $sub_structure, $prefix . ($index + 1));
      $parts = array_merge($parts, $sub_parts);
    }
    return $parts;
  }

  // Per Part Parsing.

  // Initalize file object like part structure.
  $part = new StdClass();
  $part->attributes = array();
  $part->filename = 'unnamed_attachment';
  if (!$part->filemime = mailfeeds_get_mime_type($structure)) {
    watchdog('mailhandler', 'Could not fetch mime type for message part. Defaulting to application/octet-stream.', array(), WATCHDOG_NOTICE);
    $part->filemime = 'application/octet-stream';
  }

  if ($structure->ifparameters) {
    foreach ($structure->parameters as $parameter) {
      switch (strtoupper($parameter->attribute)) {
        case 'NAME':
        case 'FILENAME':
          $part->filename = $parameter->value;
          break;
        default:
          // put every thing else in the attributes array;
          $part->attributes[$parameter->attribute] = $parameter->value;
      }
    }
  }

  // Handle Content-Disposition parameters for non-text types.
  if ($structure->type != TYPETEXT && $structure->ifdparameters) {
    foreach ($structure->dparameters as $parameter) {
      switch (strtoupper($parameter->attribute)) {
        case 'NAME':
        case 'FILENAME':
          $part->filename = $parameter->value;
          break;
        // put every thing else in the attributes array;
        default:
          $part->attributes[$parameter->attribute] = $parameter->value;
      }
    }
  }

  // Retrieve part and convert MIME encoding to UTF-8
  if(!$part->data = imap_fetchbody($stream, $msg_number, $part_number, FT_UID)) {
    watchdog('mailhandler', 'No Data!!', array(), WATCHDOG_ERROR);
    return $parts;
  }

  // Decode as necessary.
  if ($structure->encoding == ENCBASE64) {
    $part->data = imap_base64($part->data);
  }
  elseif ($structure->encoding == ENCQUOTEDPRINTABLE) {
    $part->data = quoted_printable_decode($part->data);
  }
  // Convert text attachment to UTF-8.
  elseif ($structure->type == TYPETEXT) {
    $part->data = imap_utf8($part->data);
  }

  //always return an array to satisfy array_merge in recursion catch, and array return value.
  $parts[] = $part;
  return $parts;
}

/**
 * Retrieve MIME type of the message structure.
 */
function mailfeeds_get_mime_type(&$structure) {
  static $primary_mime_type = array('TEXT', 'MULTIPART', 'MESSAGE', 'APPLICATION', 'AUDIO', 'IMAGE', 'VIDEO', 'OTHER');
  $type_id = (int)$structure->type;
  if (isset($primary_mime_type[$type_id]) && !empty($structure->subtype)) {
    return $primary_mime_type[$type_id] .'/'. $structure->subtype;
  }
  return 'TEXT/PLAIN';
}