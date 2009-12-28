<?php
// $Id:$

/**
 * Fetches data via HTTP.
 */
class MailFeedsFetcher extends FeedsFetcher {

  /**
   * Fetch a resource via POP or IMAP.
   *
   * @param $resource
   *   A resource description of type FeedsResource.
   *
   * @return
   *   A string from the requested location if successful, or FALSE if not.
   */
  public function fetch(FeedsSource $source) {
    $mailbox = $source->getConfigFor($this);
    //module_load_include('inc', 'mailfeeds', 'mailfeeds');
    //return new MailFeedsImportBatch(NULL, $result);
    return new MailFeedsImportBatch(NULL, $mailbox);
  }

  /**
   * Clear caches.
   */
  public function clear(FeedsSource $source) {
    // TODO implement for mailhandler
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {    
    $form = array();
    $form = $this->configForm(&$form_state);
    foreach ($form as $name => $item) {
      $form[$name]['#default_value'] = $source_config[$name] ? $source_config[$name] : NULL;
    }
    return $form;
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array(
      'mail' => 'foo@foo.xyz',
      'mailto' => 'foo',
      'folder' => 'bar',
      'imap' => 'baz',
    );
  }

  /**
   * Configuration form.
   */
  public function configForm(&$form_state) {
    $form = array();
    $form['mail'] = array(
      '#type' => 'textfield',
      '#title' => t('E-mail address'),
      '#default_value' => $this->config['mail'],
      '#description' => t('The e-mail address to which users should send their submissions.'),
    );
    $form['mailto'] = array(
      '#type' => 'textfield',
      '#title' => t('Second E-mail address'),
      '#default_value' => $this->config['mailto'],
      '#description' => t('Optional. The e-mail address to which modules should send generated content.'),
    );
    $form['folder'] = array(
      '#type' => 'textfield',
      '#title' => t('Folder'),
      '#default_value' => $this->config['folder'],
      '#description' => t('Optional. The folder where the mail is stored. If you want this mailbox to read from a local folder, give the full path. Leave domain, port, name, and pass empty below. Remember to set the folder to readable and writable by the webserver.'),
    );
    $form['imap'] = array(
      '#type' => 'select',
      '#title' => t('POP3 or IMAP Mailbox'),
      '#options' => array('POP3', 'IMAP'),
      '#default_value' => $this->config['imap'],
      '#description' => t('If you wish to retrieve mail from a POP3 or IMAP mailbox instead of a Folder, select POP3 or IMAP. Also, complete the Mailbox items below.'),
    );
    $form['domain'] = array(
      '#type' => 'textfield',
      '#title' => t('Mailbox domain'),
      '#default_value' => $this->config['domain'],
      '#description' => t('The domain of the server used to collect mail.'),
    );
    $form['port'] = array(
      '#type' => 'textfield',
      '#title' => t('Mailbox port'),
      '#size' => 5, '#maxlength' => 5,
      '#default_value' => $this->config['port'],
      '#description' => t('The port of the mailbox used to collect mail (usually 110 for POP3, 143 for IMAP).'),
    );
    $form['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Mailbox username'),
      '#default_value' => $this->config['name'],
      '#description' => t('This username is used while logging into this mailbox during mail retrieval.'),
    );
    $form['pass'] = array(
      '#type' => 'textfield',
      '#title' => t('Mailbox password'),
      '#default_value' => $this->config['pass'],
      '#description' => t('The password corresponding to the username above. Consider using a non-vital password, since this field is stored without encryption in the database.'),
    );
    // Allow administrators to configure the mailbox with extra IMAP commands (notls, novalidate-cert etc.)
    $form['extraimap'] = array(
      '#type' => 'textfield',
      '#title' => t('Extra commands'),
      '#default_value' => $this->config['extraimap'],
      '#description' => t('Optional. In some circumstances you need to issue extra commands to connect to your mail server (e.g. "/notls", "/novalidate-cert" etc.). See documentation for <a href="http://php.net/imap_open">imap_open</a>. Begin the string with a "/", separating each subsequent command with another "/".'),
    );
    $form['mime'] = array(
      '#type' => 'select',
      '#title' => t('Mime preference'),
      '#options' => array(
        'TEXT/HTML,TEXT/PLAIN' => 'HTML',
        'TEXT/PLAIN,TEXT/HTML' => t('Plain text'),
      ),
      '#default_value' => $this->config['mime'],
      '#description' => t('When a user sends an e-mail containing both HTML and plain text parts, use this part as the node body.'),
    );
    $form['security'] = array(
      '#type' => 'radios',
      '#title' => t('Security'),
      '#options' => array(t('Disabled'), t('Require password')),
      '#default_value' => isset($this->config['security']) ? $this->config['security'] : 0,
      '#description' => t('Disable security if your site does not require a password in the Commands section of incoming e-mails. Note: Security=Enabled and Mime preference=HTML is an unsupported combination.')
    );
    $form['replies'] = array(
      '#type' => 'radios', 
      '#title' => t('Send error replies'), 
      '#options' => array(t('Disabled'), t('Enabled')), 
      '#default_value' => $this->config['replies'], 
      '#description' => t('Send helpful replies to all unsuccessful e-mail submissions. Consider disabling when a listserv posts to this mailbox.')
    );
    $form['fromheader'] = array(
      '#type' => 'textfield', 
      '#title' => t('From header'), 
      '#default_value' => $this->config['fromheader'], 
      '#description' => t('Use this e-mail header to determine the author of the resulting node. Admins usually leave this field blank (thus using the <strong>From</strong> header), but <strong>Sender</strong> is also useful when working with listservs.')
    );
    $form['commands'] = array(
      '#type' => 'textarea', 
      '#title' => t('Default commands'), 
      '#default_value' => $this->config['commands'], 
      '#description' => t('A set of commands which are added to each message. One command per line. See <a href="@link">Commands</a>.', array('@link' => url('admin/help/mailhandler/#commands')))
    );
    $form['sigseparator'] = array(
      '#type' => 'textfield', 
      '#title' => t('Signature separator'), 
      '#default_value' => $this->config['sigseparator'], 
      '#description' => t('All text after this string will be discarded. A typical value is <strong>"-- "</strong> that is two dashes followed by a blank in an otherwise empty line. Leave blank to include signature text in nodes.')
    );
    $form['delete_after_read'] = array(
      '#type' => 'checkbox', 
      '#title' => t('Delete messages after they are processed?'), 
      '#default_value' => $this->config['delete_after_read'], 
      '#description' => t('Uncheck this box to leave read messages in the mailbox. They will not be processed again unless they become marked as unread.  If you selected "POP3" as your mailbox type, you must check this box.')
    );
    $form['enabled'] = array(
      '#type' => 'radios', 
      '#title' => t('Cron processing'), 
      '#options' => array(t('Disabled'), t('Enabled')), 
      '#default_value' => $this->config['enabled'], 
      '#description' => t('Select disable to temporarily stop cron processing for this mailbox.')
    );
    // Allow administrators to select the format of saved nodes/comments
    $form['format'] = filter_form($this->config['format']);

    return $form;

  }
}

class MailFeedsImportBatch extends FeedsImportBatch {
  /*
   * Override of getRaw()
   */
  public function getRaw($mailbox) {
    // Connect to the mailbox and return an IMAP resource result
    // TODO error handling
    module_load_include('inc', 'mailfeeds', 'mailfeeds');
    $result = mailfeeds_open_mailbox($mailbox);
    return $result;
  }
}

