<?php

/**
 * @file
 * Handling administration action in the system
 *
 * All configuration processing will be handled
 * by this module
 */

function mosquefinance_admin_menu(&$items) {
  $items['admin/config/dncsystem/mosquefinance'] = array(
    'title' => 'Mosque Finance',
    'description' => 'Settings for DNC\'s System to handling mosque finance',
    'position' => 'right',
    'weight' => 0,
    'page callback' => 'system_admin_menu_block_page',
    'access arguments' => array('administer site configuration'),
    'file' => 'system.admin.inc',
    'file path' => drupal_get_path('module', 'system'),
    );
  $items['admin/config/dncsystem/mosquefinance/global'] = array(
    'title' => 'Global Params',
    'description' => 'Stock management parameters and configurations',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mosquefinance_admin_params'),
    'file' => 'mosquefinance.admin.inc',
  );
}

/**
 * Configure tiny accounting of assets parameters
 *
 * @param array $form
 *   variable array of form
 * @param array $form_state
 *   variable array of form_state
 *
 * @return array
 *   form array that will rendered by drupal_get_form
 */
function mosquefinance_admin_params(array $form = array(), array $form_state = array()) {
  $default = variable_get('mosquefinance_configs', []);
  $form['info'] = [
    '#type' => 'item',
    '#weight' => 5,
    '#markup' => t('It\'s parameter just for the client of drupal services. Not working for another framework'),
  ];
  $form['stock'] = [
    '#type' => 'fieldset',
    '#title' => t('Account Configuration Parameters'),
    '#weight' => 20,
    '#collapsible' => TRUE,
    '#collapsed' => empty($default) ? FALSE : TRUE,
  ];
  
  $form['stock']['accounts'] = [
    '#type' => 'fieldset',
    '#title' => t('Fixed Accounts Configurations'),
    '#weight' => 10,
    '#collapsible' => TRUE,
  ];
  $setaccounts = mosquefinance_get_fixed_account_names();
  foreach ($setaccounts as $keys => $values) {
    $vtitle = str_replace('_', ' ', str_replace('-', ' ', $values));
    $form['stock']['accounts']['dnc_configs[accounts][' . $values . ']'] = [
      '#type' => 'textfield',
      '#weight' => 10 + $keys,
      '#title' => t('@acnt Account', ['@acnt' => ucwords(strtolower($vtitle))]) . ' *',
      '#description' => t('Account that represented to @title', ['@title' => $vtitle]),
      '#autocomplete_path' => 'dnctinyaccounting/search-autocomplete/accounts',
    ];
    unset ($vtitle);
    if (!empty($default['accounts'][$values])) {
      $account = dncfilewrapper_get_data('accounts', $default['accounts'][$values]);
      if (!empty($account['nid'])) {
        $account = $account['title'] . '; ' . $account['description'] . ' | ' . $account['nid'];
        $form['stock']['accounts']['dnc_configs[accounts][' . $values . ']']['#default_value'] = $account;
      }
      unset ($account);
    }
  }
  unset ($setaccounts);
  
  $form['dnc_configs[hijr_adjustment]'] = [
    '#type' => 'select',
    '#weight' => 20,
    '#title' => t('Hijr Adjustment'),
    '#description' => t('The adjustment of the hijri calendar in Ramadhan that sometimes is not the same between the calculation of the system with the determination of ru\'yatul hilal'),
    '#options' => [
      -5 => '-5',
      -4 => '-4',
      -3 => '-3',
      -2 => '-2',
      -1 => '-1',
      0 => '0',
      1 => '+1',
      2 => '+2',
      3 => '+3',
      4 => '+4',
      5 => '+5'
    ],
    '#field_suffix' => t('days'),
    '#default_value' => empty($default['hijr_adjustment']) ? 0 : $default['hijr_adjustment']
  ];
  
  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('save'),
    '#weight' => 500,
  ];
  return $form;
}

function mosquefinance_admin_params_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);

  $form_state['redirect'] = current_path();
  if (!empty($form_state['input']['dnc_configs'])) {
    if (!empty($form_state['input']['dnc_configs']['accounts'])) {
      $setaccounts = mosquefinance_get_fixed_account_names();
      $dump = [];
      foreach ($form_state['input']['dnc_configs']['accounts'] as $keys => $values) {
        if (!in_array($keys, $setaccounts)) {
          continue;
        }
        $account = $values;
        $account = explode('|', $account);
        if (!empty($account[count($account) - 1])) {
          $account = trim($account[count($account) - 1]);
          settype($account, 'int');
          $dump[$keys] = $account;
        }
        unset ($account);
      }
      unset($setaccounts);
      $form_state['input']['dnc_configs']['accounts'] = $dump;
      unset ($dump);
    }
    variable_set('mosquefinance_configs', $form_state['input']['dnc_configs']);
  }
  drupal_set_message(t('configurations have been successfully saved :)'));
}

