<?php

/**
 * @file
 * Handling administration action in the system
 *
 * All configuration processing will be handled
 * by this module
 */

function dnctinyaccounting_admin_menu(&$items) {
  $items['admin/config/dncsystem/dnctinyaccounting'] = array(
    'title' => 'Tiny Accounting',
    'description' => 'Settings for DNC\'s System to handling tiny accounting',
    'position' => 'right',
    'weight' => 0,
    'page callback' => 'system_admin_menu_block_page',
    'access arguments' => array('administer site configuration'),
    'file' => 'system.admin.inc',
    'file path' => drupal_get_path('module', 'system'),
    );
  $items['admin/config/dncsystem/dnctinyaccounting/global'] = array(
    'title' => 'Global Params',
    'description' => 'Stock management parameters and configurations',
    'access arguments' => array('administer site configuration'),
    'page callback' => 'drupal_get_form',
    'page arguments' => array('dnctinyaccounting_admin_params'),
    'file' => 'dnctinyaccounting.admin.inc',
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
function dnctinyaccounting_admin_params(array $form = array(), array $form_state = array()) {
  $default = variable_get('dnctinyaccounting_configs', []);
  $form['info'] = [
    '#type' => 'item',
    '#weight' => 5,
    '#markup' => t('It\'s parameter just for the client of drupal services. Not working for another framework'),
  ];
  $form['stock'] = [
    '#type' => 'fieldset',
    '#title' => t('Stock Configuration Parameters'),
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
  return $form;
}

function dnctinyaccounting_admin_params_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);

  $form_state['redirect'] = current_path();
  if (!empty($form_state['input']['dnc_configs'])) {
    variable_set('dnctinyaccounting_configs', $form_state['input']['dnc_configs']);
  }
  drupal_set_message(t('configurations have been successfully saved :)'));
}

