<?php
// $Id:$

/**
 * Parse POP and IMAP mailboxes
 */
class MailFeedsParser extends FeedsParser {

  /**
   * Parses a IMAP resource
   * 
   * TODO
   *   - implement imap_close to close the resource
   *     - handling whether or not to delete messages
   *   - implement per-message authentication (plugin/hook)
   *   - implement custom deletion/closing (plugin/hook)
   *   - put $items into correct form for processing
   *   - should authentication happen during processing and not parsing (when implemented) ?
   * 
   */
  public function parse(FeedsImportBatch $batch, FeedsSource $source) {
    // Take the mailbox settings, pass on to getRaw
    // getRaw will connect to the mailbox and return an imap resource
    $items = array();
    $result = $batch->getRaw($batch->getFilePath());
    $mailbox = array_shift($source->getConfig());
    if (($num = mailfeeds_get_unread_messages($result)) && !empty($num)) {
      while (count($num) != 0) {
        $i = array_shift($num);
        $items[] = mailfeeds_retrieve_message($result, $i, $mailbox);
      }
    }
    has_krumo();
    krumo($items);
  }
  
  /**
   * Return mapping sources.
   *
   * At a future point, we could expose data type information here,
   * storage systems like Data module could use this information to store
   * parsed data automatically in fields with a correct field type.
   */
  public function getMappingSources() {
    ; // TODO
  }
}