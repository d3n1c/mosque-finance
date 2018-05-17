<?php

function mosquefinance_reference_menu(&$items) {
  $items['mosquefinance/references/%mosquefinance'] = [
    'title' => 'Reference',
    'description' => 'Reference of Mosque Finance',
    'access arguments' => ['view mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_references', 2],
    'file' => 'mosquefinance.reference.inc',
  ];
  $items['mosquefinance/references/%mosquefinance/add'] = [
    'title' => 'Add',
    'description' => 'Adding reference of mosque finance',
    'access arguments' => ['administer mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_reference_add', 2],
    'file' => 'mosquefinance.reference.inc',
    'type' => MENU_LOCAL_ACTION,
  ];
  $items['mosquefinance/references/%mosquefinance/%'] = [
    'title' => 'Edit',
    'description' => 'Editing reference of mosque finance',
    'access arguments' => ['view mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_reference_edit', 2, 3],
    'file' => 'mosquefinance.reference.inc',
    'type' => MENU_VISIBLE_IN_BREADCRUMB,
  ];
}

function mosquefinance_references(array $form = [], array $form_state = [], $type) {
  if (!mosquefinance_structures_of_data($type)
    || !mosquefinance_listing_view($type)
    || !in_array($type, ['entities'])) {
    drupal_set_message(t('The type is not valid'), 'error', FALSE);
    return $form;
  }

  $refname = str_replace('_', '', str_replace('-', ' ', $type));
  $refname = t($refname);

  if (!empty($form_state['storage']['confirmed'])) {
    foreach ($form_state['storage'] as $keys => $values) {
      $form[$keys] = [
        '#type' => 'value',
        '#value' => $values
      ];
    }

    $question = t('Are you sure to delete all choosed items');
    $path = current_path();
    $yes = t('Yes Delete All');
    $no = t('Cancel');
    $description = [t('The action will confirmed and execute delete all items you\'ve choosed below')];
    $description[] = '<br /><ul>';

    foreach ($form_state['storage']['items'] as $keys => $values) {
      $data = dncfilewrapper_get_data($type, $values);
      $description[] = '<li>' . $data['title'] . (empty($data['code']) ? NULL : ' (' . $data['code'] . ')') . '</li>';
      unset ($data);
    }

    $description[] = '</ul><br />';
    $description = implode("\n", $description);

    return confirm_form($form, $question, $path, $description, $yes, $no);
  }

  $count = dncfilewrapper_get_listing($type, [], [], TRUE);
  
  if (empty($count)) {
    drupal_set_message(t('No data available yet'), 'error', FALSE);
    return $form;
  }
  
  $form['type'] = [
    '#type' => 'value',
    '#value' => $type
  ];
  
  $limit = 10;
  $current = pager_default_initialize($count, $limit);
  $current = $current * $limit;
  $data = dncfilewrapper_get_listing($type, [], [$current, $limit], FALSE, TRUE);
  unset ($current, $limit);
  
  $form['items'] = [
    '#type' => 'tableselect',
    '#title' => t('Reference of @type', ['@type' => $refname]),
    '#weight' => 10,
    '#empty' => t('No data available yet'),
    '#header' => [],
    '#options' => [],
  ];
  unset ($refname);

  $listview = mosquefinance_listing_view($type);
  foreach ($listview as $values) {
    $title = str_replace('_', ' ', str_replace('-', ' ', $values));
    $title = t($title);
    $form['items']['#header'][$values]['data'] = ucwords(strtolower($title));
    if ($values == 'normal') {
      $form['items']['#header'][$values]['style'] = 'text-align: center';
    }
    unset ($title);
  }
  
  $structure = mosquefinance_structures_of_data($type);
  foreach ($data as $keys => $values) {
    foreach ($listview as $key => $value) {
      if (isset($values[$value])) {
        if ($structure[$value]['type'] == 'reference') {
          $ref = dncfilewrapper_get_data($structure[$value]['reference'], $values[$value]);
          if (empty($ref)) {
            unset ($ref);
            continue;
          }
          $values[$value] = $ref['title'];
          unset ($ref);
        }
      }
      else {
        $values[$value] = NULL;
      }
      $form['items']['#options'][$values['nid']][$value]['data'] = $value == 'title' ? l($values[$value], 'mosquefinance/references/' . $type . '/' . $values['nid']) : (empty($values[$value]) ? NULL : (is_numeric($values[$value]) ? number_format($values[$value], 0, ',', '.') : $values[$value]));
    }
  }
  unset ($listview, $structure);

  if (user_access('administer mosque finance')) {
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => t('Delete Choosed'),
      '#weight' => 100,
      '#attributes' => ['class' => ['btn', 'btn-danger']],
    ];
  }
  else {
    $table = [];
    $table['header'] = $form['items']['#header'];
    $table['rows'] = $form['items']['#options'];
    $form['items']['#type'] = 'item';
    $form['items']['#markup'] = theme('table', $table);
    unset ($table, $form['items']['#empty'], $form['items']['#options'], $form['items']['#header']);
  }

  $form['pager'] = [
    '#type' => 'item',
    '#weight' => 500,
    '#markup' => theme('pager', ['quantity' => 10]),
  ];

  return $form;
}

function mosquefinance_references_validate(&$form, &$form_state) {
  if (empty($form_state['values']['confirmed'])) {
    foreach ($form_state['input']['items'] as $keys => $values) {
      if (!empty($values)) {
        $form_state['storage']['items'][] = $keys;
      }
    }
    if (empty($form_state['storage']['items'])) {
      drupal_set_message(t('No choosed items found'), 'error', FALSE);
      $form_state['redirect'] = current_path();
      return;
    }
    $form_state['storage']['type'] = $form_state['values']['type'];
    $form_state['storage']['confirmed'] = TRUE;
    return;
  }
  else {
    if (empty($form_state['values']['items'])) {
      drupal_set_message(t('No choosed items found'), 'error', FALSE);
      $form_state['redirect'] = current_path();
      return;
    }
  }
}

function mosquefinance_references_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  if (empty($form_state['values']['confirmed'])) {
    $form_state['rebuild'] = TRUE;
    return;
  }
  $form_state['redirect'] = current_path();
  // echo '<pre>'; print_r($form_state['values']); echo '</pre>'; exit;
  $deletes = [];
  foreach ($form_state['values']['items'] as $values) {
    $deletes[$form_state['values']['type']][] = ['nid' => $values];
  }
  $result = dncfilewrapper_delete_data($GLOBALS['user']->uid, $deletes);
  unset ($deletes);
  if (empty($result)) {
    unset ($result);
    drupal_set_message(t('Deleting items failed'), 'error', FALSE);
    return;
  }
  unset ($result);
  drupal_set_message(t('Your choosed items have been deleted'));
}

function mosquefinance_reference_add(array $form = array(), array $form_state = array(), $type) {
  if (!user_access('administer mosque finance')) {
    drupal_set_message(t('you\'re not authorized to access this page'), 'error', FALSE);
    return $form;
  }
  $form_state['storage']['type'] = $type;
  return mosquefinance_reference_update($form, $form_state);
}

function mosquefinance_reference_edit(array $form = array(), array $form_state = array(), $type, $nid) {
  $data = dncfilewrapper_get_data($type, $nid);
  if (empty($data)) {
    unset ($data);
    drupal_set_message(t('No data found by the type and or ID'), 'error', FALSE);
    return $form;
  }

  foreach ($data as $keys => $values) {
    $form_state['storage'][$keys] = $values;
  }

  $form_state['storage']['type'] = $type;
  $form_state['storage']['nid'] = $nid;
  return mosquefinance_reference_update($form, $form_state);
}

function mosquefinance_reference_update(&$form, &$form_state) {
  $breadcrumb = array();
  $breadcrumb[] = l(t('home'), '<front>');
  $pmenu = menu_link_get_preferred('mosquefinance/references/' . $form_state['storage']['type']);
  $breadcrumb[] = l(t($pmenu['link_title']), $pmenu['link_path']);
  unset ($pmenu);
  drupal_set_breadcrumb($breadcrumb);
  unset ($breadcrumb);

  if (empty($form_state['storage']['type'])) {
    drupal_set_message(t('No type found'), 'error', FALSE);
    return $form;
  }
  $type = $form_state['storage']['type'];

  if (!mosquefinance_structures_of_data($type)
    || !mosquefinance_listing_view($type)
    || !in_array($type, array('entities'))) {
    unset ($type);
    drupal_set_message(t('The type is not valid'), 'error', FALSE);
    return $form;
  }

  $structure = mosquefinance_structures_of_data($type);
  if (empty($structure)) {
    unset ($structure, $type);
    drupal_set_message(t('No structure found by the type'), 'error', FALSE);
    return $form;
  }

  $default = $form_state['storage'];
  foreach ($form_state['storage'] as $keys => $values) {
    if (!in_array($keys, array('type', 'nid'))) {
      continue;
    }
    $form[$keys] = [
      '#type' => 'value',
      '#value' => $values
    ];
  }

  if (!user_access('administer mosque finance')) {
    $locked = TRUE;
    if (!empty($form_state['storage']['nid'])) {
      drupal_set_title('Show Detail', PASS_THROUGH);
    }
  }

  $i = 1;
  foreach ($structure as $keys => $values) {
    $title = str_replace('_', ' ', str_replace('-', ' ', $keys));
    $form[$keys] = [
      '#title' => t($title),
      '#weight' => $i * 10,
      '#type' => 'textfield',
    ];
    $i++;
    unset ($title);

    if (isset($default[$keys])) {
      $form[$keys]['#default_value'] = $default[$keys];
      if (!empty($locked)) {
        $form[$keys]['#type'] = 'item';
        if ($values['type'] == 'reference') {
          $refdata = dncfilewrapper_get_data($values['reference'], $default[$keys]);
          $default[$keys] = empty($refdata['title']) ? NULL : $refdata['title'];
          unset ($refdata);
        }
        if ($keys == 'normal') {
          $default[$keys] = empty($default[$keys]) ? t('debit') : t('credit');
        }
        $form[$keys]['#markup'] = $default[$keys];
        unset ($form[$keys]['#default_value']);
        continue;
      }
    }

    if (!empty($values['required'])) {
      $form[$keys]['#required'] = TRUE;
    }
    if ($values['type'] == 'date') {
      if (!module_exists('date_popup')) {
        $form[$keys]['#size'] = 10;
        $form[$keys]['#maxlength'] = 10;
      }
      else {
        $form[$keys]['#type'] = 'date_popup';
        $form[$keys]['#date_format'] = 'Y-m-d';
        $form[$keys]['#date_label_position'] = 'none';
        $yearrange = empty($values['year_range']) ? [0, 0] : $values['year_range'];
        $ynow = date('Y', time());
        $form[$keys]['#date_year_range'] = $yearrange[0] . ':' . $yearrange[1];
        unset ($yearrange, $ynow);
      }
    }
    elseif (in_array($values['type'], array('integer', 'decimal'))) {
      $form[$keys]['#size'] = 10;
    }
    elseif ($values['type'] == 'reference') {
      if (!empty($values['hidden_in_form'])) {
        $form[$keys]['#type'] = 'value';
      }
      else {
        $widget = empty($values['widget']) ? 'select' : $values['widget'];
        if ($widget == 'select') {
          $form[$keys]['#type'] = 'select';
          $options = [];
          $data_options = dncfilewrapper_get_listing($values['reference']);
          if (!empty($data_options)) {
            foreach ($data_options as $key => $value) {
              $options[$value['nid']] = $value['title'] . (empty($value['code']) ? NULL : ' (' . $value['code'] . ')');
            }
          }
          unset ($data_options);
          $form[$keys]['#options'] = $options;
          unset ($options);
        }
        else {
          $form[$keys]['#autocomplete_path'] = 'mosquefinance/search-autocomplete/' . $values['reference'];
        }
        unset ($widget);
      }
    }

    if ($keys == 'active') {
      $form[$keys]['#type'] = 'checkbox';
    }
  }
  unset ($i);
  unset ($structure);
  unset ($default);
  unset ($type);

  if (empty($locked)) {
    $form['submit'] = [
      '#type' => 'submit',
      '#weight' => 500,
      '#value' => t('save'),
    ];
  }
  unset ($locked);
  return $form;
}

function mosquefinance_reference_update_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $destination = empty($_GET['destination']) ? 'mosquefinance/references/' . $form_state['values']['type'] : $_GET['destination'];
  $form_state['redirect'] = $destination;
  unset ($destination);

  $data = [];
  $nid = empty($form_state['values']['nid']) ? NULL : $form_state['values']['nid'];
  if (!empty($nid)) {
    $data = dncfilewrapper_get_data($form_state['values']['type'], $nid);
    if (empty($data)) {
      drupal_set_message(t('No data found by the ID'), 'error', FALSE);
      unset ($data, $nid);
      return;
    }
  }
  unset ($nid);

  $structure = mosquefinance_structures_of_data($form_state['values']['type']);
  foreach ($structure as $keys => $values) {
    if (in_array($values['type'], array('integer', 'decimal'))) {
      $numeric = preg_replace("/[^0-9\.]/", '', $form_state['input'][$keys]);
      if ($values['type'] == 'integer') {
        settype($numeric, 'int');
      }
      else {
        settype($numeric, 'float');
        $divisor = empty($values['decimal_divisor']) ? [5, 2] : $values['decimal_divisor'];
        $comma = round($numeric, $divisor[1]);
        $numdiv = explode('.', $comma);
        $numeric = (strlen($numdiv[0]) + $divisor[1]) < $divisor[0] ? $comma : $numdiv[0];
        if ($numeric !== $comma) {
          settype($numeric, 'int');
        }
        unset ($numdiv, $comma, $divisor);
      }
      $form_state['input'][$keys] = $numeric;
      unset ($numeric);
    }
    elseif ($values['type'] == 'reference') {
      $widget = empty($values['widget']) ? 'select' : $values['widget'];
      if ($widget == 'autocomplete') {
        $refval = explode('|', $form_state['input'][$keys]);
        $form_state['input'][$keys] = trim($refval[count($refval) - 1]);
        unset ($refval);
      }
      unset ($widget);
    }
    elseif ($values['type'] == 'date') {
      $form_state['input'][$keys] = empty($form_state['input'][$keys]['date']) ? $form_state['input'][$keys] : $form_state['input'][$keys]['date'];
    }
    if (isset($form_state['input'][$keys])) {
      $data[$keys] = $form_state['input'][$keys];
    }
  }
  unset ($structure);

  $data = mosquefinance_set_data_type($form_state['values']['type'], $data);
  $result = dncfilewrapper_update_data($GLOBALS['user']->uid, [$form_state['values']['type'] => [$data]]);
  unset ($data);
  if (empty($result)) {
    drupal_set_message(t('Failed on data saving'), 'error', FALSE);
    return;
  }
  drupal_set_message(t('The data has been saved'));
}

function mosquefinance_reference_add_submit(&$form, &$form_state) {
  mosquefinance_reference_update_submit($form, $form_state);
}

function mosquefinance_reference_edit_submit(&$form, &$form_state) {
  mosquefinance_reference_update_submit($form, $form_state);
}
