<?php

class MailFeedImporter extends FeedsImporter {
  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = array();
    $form['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#description' => t('The name of this configuration.'),
      '#default_value' => $this->config['name'],
      '#required' => TRUE,
    );
    $form['description'] = array(
      '#type' => 'textfield',
      '#title' => t('Description'),
      '#description' => t('A description of this configuration.'),
      '#default_value' => $this->config['description'],
    );
    $form['content_type'] = array(
      '#type' => 'select',
      '#title' => t('Attach to content type'),
      '#description' => t('If you attach a configuration to a node you can use nodes for creating feeds on your site.'),
      '#options' => array('' => t('Use standalone form')) + node_get_types('names'),
      '#default_value' => $this->config['content_type'],
    );
    $period = drupal_map_assoc(array(0, 900, 1800, 3600, 10800, 21600, 43200, 86400, 259200, 604800, 2419200), 'format_interval');
    $period[FEEDS_SCHEDULE_NEVER] = t('Never');
    $period[0] = t('As often as possible');
    $form['import_period'] = array(
      '#type' => 'select',
      '#title' => t('Minimum refresh period'),
      '#options' => $period,
      '#description' => t('This is the minimum time that must elapse before a feed may be refreshed automatically.'),
      '#default_value' => $this->config['import_period'],
    );
    $form['import_on_create'] = array(
      '#type' => 'checkbox',
      '#title' => t('Import on create'),
      '#description' => t('Check if content should be imported at the moment of feed submission.'),
      '#default_value' => $this->config['import_on_create'],
    );
    return $form;
  }
}