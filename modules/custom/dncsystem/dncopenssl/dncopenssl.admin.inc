<?php

// $Id: dncopenssl, v 0.1.0.0 Apr 5, 2014 6:12:04 AM denic Exp $

/*
 *  @file
 *  install file for dncopenssl
 *
 *  Copyright (c) 2013 Denic Wibowo<denicwibowo@gmail.com>.
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *  Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  dncopenssl.admin.inc
 *
 *  Created on Apr 5, 2014, 6:12:04 AM
 */

/**
 *
 * @return type
 */
function dncopenssl_params_configuration () {
  $publicKey = dncopenssl_get_public_key();
  if (!empty($publicKey)) {
    drupal_set_message(t('Public Key is <br /><br />@pubkey', array('@pubkey' => $publicKey)), 'status', FALSE);
  }
  $form = array();
  $form['dncopenssl_key_path'] = array(
    '#type' => !empty($publicKey) ? 'item' : 'textfield',
    '#title' => t('Key Path'),
    '#description' => t('Path of local filesystem to store it\'s key, (use absolute path pleasee... )'),
    '#size' => 20,
    '#weight' => 10,
    '#markup' => variable_get('dncopenssl_key_path'),
    '#default_value' => variable_get('dncopenssl_key_path'),
    '#required' => !empty($publicKey) ? FALSE : TRUE,
  );
  $default = variable_get('dncopenssl_privat_name');
  $form['dncopenssl_privat_name'] = array(
    '#type' => empty($default) ? 'textfield' : 'item',
    '#size' => 40,
    '#required' => empty($default) ? TRUE : FALSE,
    '#weight' => 20,
    '#title' => t('Private Name'),
    '#markup' => empty($default) ? NULL : $default,
    '#default_value' => empty($default) ? time() . rand(10000, 99999) : $default,
  );
  if (!empty($form['dncopenssl_key_path']['#default_value']) && empty($publicKey)) {
    drupal_set_message(t('You need to create secret key for the first time !here', array('!here' => l('here', 'dncopenssl/createkey', array(
      'query' => array(
        'destination' => current_path(),
      )
    )))), 'warning', FALSE);
  }

  if (!empty($publicKey) && !empty($default)) {
    return $form;
  }
  unset ($default);
  return system_settings_form($form);
}

/**
 *
 * @return type
 */
function dncopenssl_create_secret_key() {
  $destination = empty($_GET['destination']) ? '<front>' : $_GET['destination'];
  $publicKey = dncopenssl_get_public_key();
  if (!empty($publicKey)) {
    drupal_set_message(t('Public Key is <pre>@pubkey</pre>', array('@pubkey' => $publicKey)), 'status', FALSE);
    unset ($publicKey);
    drupal_goto($destination);
    return;
  }
  unset ($publicKey);
  $form = array();
  $form['destination'] = array(
    '#type' => 'value',
    '#value' => $destination,
  );
  return confirm_form(
          $form,
          t('Create secret key for the first time'),
          drupal_get_path_alias('<front>'),
          t('You must create secret key now'),
          t('Create now !'),
          t('Cancel')
          );
}

/**
 *
 * @global type $base_url
 * @param type $form
 * @param array $form_state
 * @return type
 */
function dncopenssl_create_secret_key_submit($form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['redirect'] = $form_state['values']['destination'];
  global $base_url;
  $domain = parse_url($base_url, PHP_URL_HOST);
  $path = variable_get('dncopenssl_key_path');
  if (empty($path)) {
    unset ($path);
    return;
  }
  $file = $path . DIRECTORY_SEPARATOR . $domain . '.key';
  $privateKey = openssl_pkey_new(
    array(
      'private_key_bits' => 1024,
      'private_key_type' => OPENSSL_KEYTYPE_RSA,
    )
  );
  openssl_pkey_export_to_file($privateKey, $file);
  unset ($file);
  $publicKey = openssl_pkey_get_details($privateKey);
  openssl_free_key($privateKey);
  unset ($privateKey);
  $publicKey = $publicKey['key'];
  $file = $path . DIRECTORY_SEPARATOR . $domain . '_public.key';
  unset ($domain);
  file_put_contents($file, $publicKey);
  drupal_set_message(t('Public Key is <pre>@pubkey</pre>', array('@pubkey' => $publicKey)), 'status', FALSE);
  unset ($publicKey);
}

/**
 *
 * @param type $form
 * @param type $form_state
 * @return boolean
 */
function dncopenssl_clients($form = array(), $form_state = array()) {
  if (!empty($form_state['storage']['clients'])) {
    $form['clients'] = array(
      '#type' => 'value',
      '#value' => $form_state['storage']['clients'],
    );
    $form['faction'] = array(
      '#type' => 'value',
      '#value' => $form_state['storage']['faction'],
    );
    $clients = '';
    foreach ($form_state['storage']['clients'] as $value) {
      if (!empty($value)) {
        $clients .= '<li>' . $value . '</li>';
      }
    }
    $action = $form_state['storage']['faction'] == 'drop' ? t('delete') : t('set');
    return confirm_form(
      $form,
      t('Are you sure to @action these clients ?', array('@action' => $action)),
      drupal_get_path_alias('admin/structure/dncsystem/dncopenssl'),
      t('<ul>!clients</ul>', array('!clients' => $clients)) . '<br />' .
      t('This action cannot be undone !'),
      t('Delete'),
      t('Cancel')
    );
  }

  $header = array(
    'clientID' => array(
      'data' => t('Client ID'),
      'field' => 'clientID',
      'width' => '15%',
      'sort' => 'asc',
    ),
    'publicKey' => array(
      'data' => t('Public Key'),
      'field' => 'publicKey',
    ),
  );

  $result = db_select('dncopenssl', 'n')
          ->extend('PagerDefault')
            ->limit(10)
          ->extend('TableSort')
            ->orderByHeader($header)
          ->fields('n');
  $result = $result->execute();
  $result = $result->fetchAll();

  if (is_array($result) && count($result) > 0) {
    $form['operation'] = array(
      '#type' => 'fieldset',
      '#title' => t('List of Clients'),
      '#description' => t('Clients with it\'s secret key that represented by built-in drupal hash salt<br />Action will execute for selected items'),
    );
    $form['operation']['faction'] = array(
      '#type' => 'select',
      '#options' => array(
        'drop' => t('Delete selected items')
      ),
      '#prefix' => '<div class="container-inline">'
    );
    $form['operation']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Execute'),
      '#suffix' => '</div>'
    );

    $options = array();
    foreach ($result as $value) {
      $options[$value->clientID] = array(
        'clientID' => l($value->clientID, drupal_get_path_alias('admin/structure/dncsystem/dncopenssl/' . $value->clientID)),
        'publicKey' => !is_null($value->publicKey) && trim($value->publicKey) ? t('<pre>@printr</pre>', array('@printr' => print_r(base64_decode($value->publicKey), TRUE))) : NULL,
      );
    }

    $form['clients'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#empty' => t('No data available'),
      '#attributes' => array('width' => '100%'),
      '#caption' => t('Application Clients'),
      '#multiple' => TRUE,
    );
    unset ($options, $header);
  }

  return $form;
}

/**
 *
 * @param type $form
 * @param type $form_state
 */
function dncopenssl_clients_validate(&$form, &$form_state) {
  if (empty($form_state['storage']['clients'])) {
    foreach ($form_state['input']['clients'] as $key => $value) {
      if (!empty($value)) {
        $ok = 1;
      }
    }
    if (!isset($ok)) {
      form_set_error('clients', t('No item to execute, please choose at least one item !'));
    }
    unset ($ok);
  }
}

/**
 *
 * @param type $form
 * @param type $form_state
 */
function dncopenssl_clients_submit(&$form, &$form_state) {
  if (empty($form_state['storage']['clients'])) {
    $form_state['rebuild'] = TRUE;
    $form_state['storage']['clients'] = $form_state['input']['clients'];
    $form_state['storage']['faction'] = $form_state['input']['faction'];
  }
  else {
    foreach ($form_state['values']['clients'] as $value) {
      if ($form_state['values']['faction'] == 'drop') {
        if (!empty($value)) {
          db_delete('dncopenssl')
          ->condition('clientID', $value)
          ->execute();
        }
      }
    }
    drupal_set_message(t('Client %clients have been delete !', array('%clients' => implode(', ', $form_state['values']['clients']))));
    $form_state['redirect'] = drupal_get_path_alias('admin/structure/dncsystem/dncopenssl');
  }
}

/**
 *
 * @return type
 */
function dncopenssl_clients_add() {
  return _dncopenssl_clients_exec();
}

/**
 *
 * @return type
 */
function dncopenssl_clients_edit() {
  $varkey = explode('/', drupal_get_path_alias($_GET['q']));
  $varkey = trim($varkey[(count($varkey) - 1)]);
  $check = dncopenssl_get_client_data($varkey);
  if (!is_null($check)) {
    $form = array();
    $form_state['storage']['opr'] = 'edit';
    foreach ($check as $key => $value) {
      $form_state['storage'][$key] = $value;
    }
    unset ($check);
    return _dncopenssl_clients_exec($form, $form_state);
  }
  unset ($check);
  drupal_set_message(t('No data avalaible for it\'s client ID'), 'error');
  drupal_goto(drupal_get_path_alias('admin/structure/dncsystem/dncopenssl'));
}

/**
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function _dncopenssl_clients_exec($form = array(), $form_state = array()) {
  if (empty($form_state['storage']['opr'])) {
    $form['clientID'] = array(
      '#type' => 'textfield',
      '#title' => t('Client ID'),
      '#description' => t('ID of client'),
      '#size' => 20,
      '#required' => TRUE,
    );
    $caption = t('Add');
  }
  else {
    $form['opr'] = array(
      '#type' => 'value',
      '#value' => $form_state['storage']['opr'],
    );
    $form['clientID'] = array(
      '#type' => 'value',
      '#value' => $form_state['storage']['clientID'],
    );

    $form['vclientID'] = array(
      '#type' => 'item',
      '#title' => t('Client ID'),
      '#description' => t('ID of client'),
      '#markup' => $form_state['storage']['clientID'],
      '#size' => 1,
    );
    $caption = t('Edit');
  }
  $form['publicKey'] = array(
    '#type' => 'textarea',
    '#title' => t('Public Key'),
    '#description' => t('Public key of client\'s ssl to send encrypted message to it !warning', array('!warning' => '<font color="red">(in <strong>base64 encoded format</strong> please...)</font>')),
    '#cols' => 25,
    '#rows' => 4,
    '#required' => TRUE,
  );
  if (!empty($form_state['storage']['publicKey'])) {
    $form['publicKey']['#default_value'] = $form_state['storage']['publicKey'];
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $caption,
  );

  unset ($options, $caption);
  return $form;
}

/**
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function dncopenssl_clients_add_submit($form, &$form_state) {
  return _dncopenssl_clients_exec_submit($form, $form_state);
}

/**
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function dncopenssl_clients_edit_submit($form, &$form_state) {
  return _dncopenssl_clients_exec_submit($form, $form_state);
}

/**
 *
 * @param type $form
 * @param array $form_state
 */
function _dncopenssl_clients_exec_submit($form, &$form_state) {
  form_state_values_clean($form_state);
  $data = $form_state['input'];
  $data['source_clientID'] = empty($form_state['values']['clientID']) ? NULL : $form_state['values']['clientID'];
  dncopenssl_update_register($data);
  unset ($data);
  drupal_set_message(t('Client %client has been @opred', array('%client' => $form_state['values']['clientID'], '@opr' => !empty($form_state['values']['opr']) ? $form_state['values']['opr'] : 'add')));
  $form_state['redirect'] = current_path();
}
