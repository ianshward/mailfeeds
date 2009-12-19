<?php
// $Id:$

/**
 * Fetches data via HTTP.
 */
class MailFeedFetcher extends FeedsFetcher {

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
    // TODO include the mailhandler library
    // TODO change mailhandler_retrieve_message to not do any processing, just fetch.
    
    $source_config = $source->getConfigFor($this);
    $url = $source_config['source'];

    feeds_include_library('http_request.inc', 'http_request');
    if ($this->config['auto_detect_feeds']) {
      $result = http_request_get_common_syndication($url);
    }
    else {
      $result = http_request_get($url);
    }
    if ($result->code != 200) {
      throw new Exception(t('Download of @url failed with code !code.', array('@url' => $url, '!code' => $result->code)));
    }
    return new FeedsFetcherResult($result->data, 'text/xml');
  }

  /**
   * Clear caches.
   */
  public function clear(FeedsSource $source) {
    // TODO implement for mailhandler
    $source_config = $source->getConfigFor($this);
    $url = $source_config['source'];
    feeds_include_library('http_request.inc', 'http_request');
    http_request_clear_cache($url);
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {
    // TODO - should mailhander provide these settings or implement them here?
    //   It seems it would make sense that mailhandler library maintain these
    //   settings, since its more aware of itself.  But, then how/where would
    //   these settings be saved?
    
    // This argument could go the other way, since mailhandler will need to
    // allow for overrides for deletion, authentication, and processing.
    // Relying on feeds for all of this could be requiring too much from
    // feeds.
    
    // Do we even need to implement this?
    
    $form = array();
    
    
    
    return $form;
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return array('auto_detect_feeds' => FALSE);
  }

  /**
   * Configuration form.
   */
  public function configForm(&$form_state) {
    $form = array();
    return $form;
    // @todo:
    $form['auto_detect_feeds'] = array(
      '#type' => 'checkbox',
      '#title' => t('Auto detect feeds (not tested)'),
      '#description' => t('Auto detect RSS or Atom feeds in the downloaded document and use them instead of the given URL.'),
      '#default_value' => $this->config['auto_detect_feeds'],
      );
    return $form;
  }
}

