<?php

function dncsystem_add_thousand_separator_js($tseparator = '.', $dseparator = ',') {
  $libraries = libraries_get_libraries();
  if (empty($libraries['autonumeric'])) {
    unset ($libraries);
    $messages = array();
    $messages[] = t('No autonumeric libraries found :(');
    $messages[] = t('You can download first this module in') . ' ' . l(t('here'), 'https://github.com/BobKnothe/autoNumeric/tree/master', array('external' => TRUE));
    $messages = implode('<br />', $messages);
    drupal_set_message($messages, 'error', FALSE);
    unset ($messages);
    return;
  }
  drupal_add_js($libraries['autonumeric'] . '/dist/autoNumeric.min.js');
  unset ($libraries);
  if ($tseparator === $dseparator) {
    $tseparator = '.';
    $dseparator = ',';
  }
  $script = "
    jQuery(function($) {
      $('#dncthousand').autoNumeric('init', {aSep: '" . $tseparator . "', aDec: '" . $dseparator . "'});
      $('#dncthousand0').autoNumeric('init', {aSep: '" . $tseparator . "', aDec: '" . $dseparator . "'});
      $('#dncthousand1').autoNumeric('init', {aSep: '" . $tseparator . "', aDec: '" . $dseparator . "'});
      $('#dncthousand2').autoNumeric('init', {aSep: '" . $tseparator . "', aDec: '" . $dseparator . "'});
    });
  ";
  drupal_add_js($script, array('type' => 'inline', 'scope' => 'footer'));
}
