<?php

function mosquefinance_action_menu(&$items) {
  $items['mosquefinance/action/beginning-balance'] = [
    'title' => 'Beginning Balance',
    'description' => 'Beginning balance transaction action',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_beginning_balance'],
    'file' => 'mosquefinance.action.inc',
  ];
  $items['mosquefinance/action/incoming'] = [
    'title' => 'Incoming',
    'description' => 'Incoming transaction action',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_incoming'],
    'file' => 'mosquefinance.action.inc',
  ];
  $items['mosquefinance/action/expending'] = [
    'title' => 'Expending',
    'description' => 'Expending transaction action',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_expending'],
    'file' => 'mosquefinance.action.inc',
  ];
  $items['mosquefinance/action/debting'] = [
    'title' => 'Debting',
    'description' => 'Debting transaction action',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_debting'],
    'file' => 'mosquefinance.action.inc',
  ];
  $items['mosquefinance/action/crediting'] = [
    'title' => 'Crediting',
    'description' => 'Crediting transaction action',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_crediting'],
    'file' => 'mosquefinance.action.inc',
  ];
  $items['mosquefinance/action/recents'] = [
    'title' => 'Recents',
    'description' => 'Recent transaction actions',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_recents'],
    'file' => 'mosquefinance.action.inc',
  ];
  $items['mosquefinance/action/recents/%mosquefinance'] = [
    'title' => 'Recent Detail',
    'description' => 'Detail of recent transaction actions',
    'access arguments' => ['use mosque finance'],
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_action_recents_edit', 3],
    'file' => 'mosquefinance.action.inc',
    'type' => MENU_VISIBLE_IN_BREADCRUMB
  ];
}

function mosquefinance_action_beginning_balance(array $form = [], array $form_state = []) {
  $data = dncfilewrapper_get_listing('balances');
  $transactions = dncfilewrapper_get_listing('transactions', [], [], TRUE);
  if ((!empty($data[0]) && count($data) > 1) || !empty($transactions) || !user_access('administer mosque finance')) {
    $locked = TRUE;
  }
  $data = !empty($data[0]) ? $data[0] : [];
  $data['balance'] = empty($data['balance']) ? 0 : $data['balance'];
  $data['date'] = empty($data['date']) ? time() : strtotime($data['date']);
  $data['date'] = date('Y-m-d', $data['date']);
  $form['date'] = [
    '#title' => t('Date'),
    '#weight' => 10,
    '#default_value' => $data['date']
  ];
  $form['balance'] = [
    '#type' => 'textfield',
    '#title' => t('Beginning Balance'),
    '#default_value' => $data['balance'],
    '#weight' => 20,
    '#size' => 10
  ];
  unset ($data);
  
  if (empty($locked)) {
    if (module_exists('date_popup')) {
      $form['date']['#type'] = 'date_popup';
      $form['date']['#date_format'] = 'Y-m-d';
      $form['date']['#date_type'] = DATE_FORMAT_DATE;
      $form['date']['#date_year_range'] = '-1:0';
      $form['date']['#date_label_position'] = 'none';
      $form['spacer'] = [
        '#type' => 'item',
        '#weight' => $form['date']['#weight'] + 0.1,
        '#markup' => ''
      ];
    }
    else {
      $form['date']['#type'] = 'textfield';
      $form['date']['#size'] = 10;
      $form['date']['#maxlength'] = 10;
      $form['date']['#description'] = t('Fill date by ISO Formatted date (YYYY-MM-DD) e.g. @date', [
        '@date' => date('Y-m-d', time())
      ]);
    }
    $form['date']['#required'] = TRUE;
    
    $form['submit'] = [
      '#type' => 'submit',
      '#weight' => 100,
      '#value' => t('save')
    ];
  }
  else {
    $form['date']['#type'] = 'item';
    $form['date']['#markup'] = dncsystem_date('j F Y', strtotime($form['date']['#default_value']));
    $form['balance']['#type'] = 'item';
    $form['balance']['#markup'] = 'Rp. ' . number_format($form['balance']['#default_value'], 0, ',', '.') . ',-';
    unset (
      $form['date']['#default_value'],
      $form['balance']['#default_value'],
      $form['balance']['#size']
    );
  }
  
  return $form;
}

function mosquefinance_action_beginning_balance_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['input']['date'] = empty($form_state['input']['date']['date']) ? $form_state['input']['date'] : $form_state['input']['date']['date'];
  $w = date('w', strtotime($form_state['input']['date']));
  if ($w != 5) {
    unset ($w);
    form_set_error('date', t('The selected date must be Friday'));
    return;
  }
  unset ($w);
  $form_state['redirect'] = current_path();
  $data = [];
  $default = dncfilewrapper_get_listing('balances');
  if (!empty($default) && count($default) > 1) {
    unset ($default, $data);
    return;
  }
  else {
    $data = $default[0];
  }
  $data['date'] = date('Y-m-d', strtotime($form_state['input']['date']));
  $data['balance'] = $form_state['input']['balance'];
  $data = mosquefinance_set_data_type('balances', $data);
  dncfilewrapper_update_data($GLOBALS['user']->uid, ['balances' => [$data]]);
  unset ($data);
  drupal_set_message(t('Beginning balance has been set'));
}

function mosquefinance_action_form(&$form, &$form_state, $action) {
  $friday = mosquefinance_get_frydate_from_now();
  $friday = date('Y-m-d', $friday);
  $balance = mosquefinance_get_balance($friday);
  if (!empty($balance)) {
    drupal_set_message(t('All transaction for next friday report have been saved and validated. New transaaction should be kept and posted after the next friday'), 'error', FALSE);
    drupal_set_message(t('You can see the next friday report') . ' ' . l(t('here'), 'mosquefinance/report/weekly', [
      'query' => [
        'date' => $friday
      ]
    ]));
    unset ($friday);
    return $form;
  }
  unset ($friday);
  
  $structure = mosquefinance_structures_of_data('proposals');
  if (!empty($form_state['storage']['confirm'])) {
    foreach ($form_state['storage'] as $keys => $values) {
      $form[$keys] = [
        '#type' => 'value',
        '#value' => $values
      ];
    }
    $question = t('Are you sure to save the transaction') . ' ?';
    $path = 'mosquefinance/action/' . $action;
    $description = [];
    $description[] = t('This action will save the transaction as shown below. Therefore check back and make sure whether the items in the transaction are in accordance with what you mean, because this action can not be undone.');
    $description[] = '<ul>';
    
    foreach ($structure as $keys => $values) {
      if ($keys == 'journals') {
        continue;
      }
      elseif ($keys == 'person_in_charge') {
        if (in_array($action, ['incoming', 'expending'])) {
          continue;
        }
      }
      $title = str_replace('_', ' ', str_replace('-', ' ', $keys));
      $title = $title == 'time' ? 'date' : $title;
      $title = ucwords(strtolower($title));
      $data = $keys == 'time' ? dncsystem_date('j F Y' , $form_state['storage'][$keys]) : $form_state['storage'][$keys];
      if ($values['type'] == 'reference') {
        $reference = dncfilewrapper_get_data($values['reference'], $data);
        $data = empty($reference['title']) ? NULL : $reference['title'];
        unset ($reference);
      }
      elseif ($keys == 'total') {
        $data = 'Rp. ' . number_format($data, 0, ',', '.') . ',-';
      }
      elseif ($keys == 'action') {
        $data = t($data);
      }
      $description[] = '<li><strong>' . t($title) . ':</strong> ' . $data . '</li>';
      unset ($title, $data);
    }
    
    $description[] = '</ul>';
    $description[] = '<br />';
    $description = implode("\n", $description);
    $yes = t('Yes Save Now');
    $no = t('Cancel');
    $form_state['storage']['confirmation'] = [
      'question' => $question,
      'path' => $path,
      'description' => $description,
      'yes' => $yes,
      'no' => $no
    ];
    return;
  }
  
  $i = 1;
  foreach ($structure as $keys => $values) {
    if ($keys == 'journals') {
      continue;
    }
    if ($keys == 'person_in_charge') {
      if (in_array($action, ['incoming', 'expending'])) {
        continue;
      }
    }
    $title = str_replace('_', ' ', str_replace('-', ' ', $keys));
    $title = $title == 'time' ? 'date' : $title;
    $title = ucwords(strtolower($title));
    $form[$keys] = [
      '#type' => 'textfield',
      '#title' => t($title),
      '#weight' => $i * 10
    ];
    if (!empty($values['required'])) {
      $form[$keys]['#required'] = TRUE;
    }
    if ($values['type'] == 'reference') {
      $form[$keys]['#type'] = 'select';
      $options = [];
      $reference = dncfilewrapper_get_listing($values['reference']);
      if (!empty($reference)) {
        foreach ($reference as $key => $value) {
          $options[$value['nid']] = $value['title'];
        }
      }
      unset ($reference);
      $form[$keys]['#options'] = $options;
      unset ($options);
    }
    if ($keys == 'time') {
      if (module_exists('date_popup')) {
        $form[$keys]['#type'] = 'date_popup';
        $form[$keys]['#date_format'] = 'Y-m-d';
        $form[$keys]['#date_type'] = DATE_FORMAT_DATE;
        $form[$keys]['#date_year_range'] = '-1:0';
        $form[$keys]['#date_label_position'] = 'none';
        $form[$keys]['#datepicker_options']['maxDate'] = 0;
        $friday = mosquefinance_get_frydate_from_now();
        $friday -= 60 * 60 * 24 * 7;
        $form[$keys]['#datepicker_options']['minDate'] = date('Y-m-d', $friday);
        unset ($friday);
      }
      else {
        $form[$keys]['#type'] = 'textfield';
        $form[$keys]['#size'] = 10;
        $form[$keys]['#maxlength'] = 10;
        $form[$keys]['#description'] .= ' ' . t('Please use UNIX standard format YYYY-MM-DD, e.g. @datenow', [
          '@datenow' => date('Y-m-d', time())
        ]);
      }
      $form[$keys]['#description'] = t('The date on which the transaction occurred');
      $form[$keys]['#default_value'] = date('Y-m-d', time());
      $form['spacer'] = [
        '#type' => 'item',
        '#markup' => ' ',
        '#weight' => $form[$keys]['#weight'] + 0.1,
      ];
    }
    elseif ($keys == 'title') {
      $form[$keys]['#title'] = t('Description');
      $form[$keys]['#description'] = t('Description of the transaction');
    }
    elseif ($keys == 'total') {
      $form[$keys]['#description'] = t('Total value of the transaction');
      $form[$keys]['#size'] = 20;
      $form[$keys]['#attributes'] = [
        'style' => 'text-align: right'
      ];
    }
    elseif ($keys == 'action') {
      unset ($form[$keys]);
    }
    unset ($title);
    $i++;
  }
  unset ($structure, $i);
  
  $form['submit'] = [
    '#type' => 'submit',
    '#weight' => 100,
    '#value' => t('save'),
    '#attributes' => [
      'class' => ['btn', 'btn-success']
    ]
  ];
}

function mosquefinance_action_form_validate(&$form, &$form_state, $action) {
  if (empty($form_state['values']['confirm'])) {
    $friday = mosquefinance_get_frydate_from_now();
    $fdate = date('Y-m-d', $friday);
    $balance = mosquefinance_get_balance($fdate);
    if (!empty($balance['date'])) {
      unset ($friday, $balance, $fdate);
      form_set_error('time', t('All transaction for next friday report have been saved and validated. New transaaction should be kept and posted after the next friday'));
      return;
    }
    unset ($fdate, $balance);
    
    $form_state['input']['time'] = !empty($form_state['input']['time']['date']) ? $form_state['input']['time']['date'] : $form_state['input']['time'];
    $strdate = strtotime($form_state['input']['time']);
    $lastfriday = $friday - (60 * 60 * 24 * 7);
    if ($strdate < $lastfriday || $strdate > $friday) {
      unset ($friday, $strdate, $lastfriday);
      form_set_error('time', t('The transaction is out of date range'));
      return;
    }
    unset ($friday, $strdate, $lastfriday);
    
    $form_state['storage']['confirm'] = TRUE;
    $form_state['storage']['action'] = $action;
    $form_state['rebuild'] = TRUE;
    
    $structure = mosquefinance_structures_of_data('proposals');
    foreach ($structure as $keys => $values) {
      if ($keys == 'journals') {
        continue;
      }
      elseif ($keys == 'total') {
        settype($form_state['input'][$keys], 'int');
        if (empty($form_state['input'][$keys])) {
          form_set_error ($keys, t('@keys value element cannot be null or must be more than 0', [
            '@keys' => ucwords(strtolower($keys))
          ]));
          return;
        }
      }
      if (!empty($form_state['input'][$keys])) {
        $form_state['storage'][$keys] = $form_state['input'][$keys];
        if ($keys == 'time') {
          $form_state['storage'][$keys] = strtotime($form_state['storage'][$keys]);
        }
      }
    }
    unset ($structure);
  }
}

function mosquefinance_action_form_submit(&$form, &$form_state, $action) {
  form_state_values_clean($form_state);
  $form_state['redirect'] = 'mosquefinance/action/recents';
  $data = [];
  $structure = mosquefinance_structures_of_data('proposals');
  foreach ($structure as $keys => $values) {
    if (!empty($form_state['values'][$keys])) {
      $data[$keys] = $form_state['values'][$keys];
    }
  }
  settype($form_state['values']['total'], 'int');
  $data['journals'] = [];
  $scenarios = mosquefinance_journal_scenarios($action);
  for ($i = 0; $i < 2; $i++) {
    $data['journals'][$i]['account'] = $scenarios[$i];
    $data['journals'][$i]['action'] = $i;
    $data['journals'][$i]['worth'] = $form_state['values']['total'];
  }
  unset ($scenarios);
  $data = mosquefinance_set_data_type('proposals', $data);
  $result = dncfilewrapper_update_data($GLOBALS['user']->uid, ['proposals' => [$data]]);
  unset ($data);
  if (empty($result)) {
    unset ($result);
    drupal_set_message(t('Failed to save transaction'), 'error', FALSE);
    return;
  }
  unset ($result);
  drupal_set_message(t('Success to save transaction'));
}

function mosquefinance_action_incoming(array $form = [], array $form_state = []) {
  mosquefinance_action_form($form, $form_state, 'incoming');
  if (!empty($form_state['storage']['confirm'])) {
    return confirm_form(
        $form,
        $form_state['storage']['confirmation']['question'],
        $form_state['storage']['confirmation']['path'],
        $form_state['storage']['confirmation']['description'],
        $form_state['storage']['confirmation']['yes'],
        $form_state['storage']['confirmation']['no']
    );
  }
  return $form;
}

function mosquefinance_action_incoming_validate(&$form, &$form_state) {
  mosquefinance_action_form_validate($form, $form_state, 'incoming');
}

function mosquefinance_action_incoming_submit(&$form, &$form_state) {
  mosquefinance_action_form_submit($form, $form_state, 'incoming');
}

function mosquefinance_action_expending(array $form = [], array $form_state = []) {
  mosquefinance_action_form($form, $form_state, 'expending');
  if (!empty($form_state['storage']['confirm'])) {
    return confirm_form(
        $form,
        $form_state['storage']['confirmation']['question'],
        $form_state['storage']['confirmation']['path'],
        $form_state['storage']['confirmation']['description'],
        $form_state['storage']['confirmation']['yes'],
        $form_state['storage']['confirmation']['no']
    );
  }
  return $form;
}

function mosquefinance_action_expending_validate(&$form, &$form_state) {
  mosquefinance_action_form_validate($form, $form_state, 'expending');
}

function mosquefinance_action_expending_submit(&$form, &$form_state) {
  mosquefinance_action_form_submit($form, $form_state, 'expending');
}

function mosquefinance_action_debting(array $form = [], array $form_state = []) {
  mosquefinance_action_form($form, $form_state, 'debting');
  if (!empty($form_state['storage']['confirm'])) {
    return confirm_form(
        $form,
        $form_state['storage']['confirmation']['question'],
        $form_state['storage']['confirmation']['path'],
        $form_state['storage']['confirmation']['description'],
        $form_state['storage']['confirmation']['yes'],
        $form_state['storage']['confirmation']['no']
    );
  }
  return $form;
}

function mosquefinance_action_debting_validate(&$form, &$form_state) {
  mosquefinance_action_form_validate($form, $form_state, 'debting');
}

function mosquefinance_action_debting_submit(&$form, &$form_state) {
  mosquefinance_action_form_submit($form, $form_state, 'debting');
}

function mosquefinance_action_crediting(array $form = [], array $form_state = []) {
  mosquefinance_action_form($form, $form_state, 'crediting');
  if (!empty($form_state['storage']['confirm'])) {
    return confirm_form(
        $form,
        $form_state['storage']['confirmation']['question'],
        $form_state['storage']['confirmation']['path'],
        $form_state['storage']['confirmation']['description'],
        $form_state['storage']['confirmation']['yes'],
        $form_state['storage']['confirmation']['no']
    );
  }
  return $form;
}

function mosquefinance_action_crediting_validate(&$form, &$form_state) {
  mosquefinance_action_form_validate($form, $form_state, 'crediting');
}

function mosquefinance_action_crediting_submit(&$form, &$form_state) {
  mosquefinance_action_form_submit($form, $form_state, 'crediting');
}

function mosquefinance_action_recents(array $form = [], array $form_state = []) {
  $listview = mosquefinance_listing_view('proposals');
  $structure = mosquefinance_structures_of_data('proposals');
  $firstbalance = mosquefinance_get_balance();
  $firstbalance['balance'] = empty($firstbalance['balance']) ? 0 : $firstbalance['balance'];
  $endbalance = $firstbalance['balance'];
  $scenarios = mosquefinance_journal_scenarios();
  
  if (!empty($form_state['storage']['confirm'])) {
    foreach ($form_state['storage'] as $keys => $values) {
      $form[$keys] = [
        '#type' => 'value',
        '#value' => $values
      ];
    }
    
    $question = t('Are you sure to delete all this items') . ' ?';
    $path = current_path();
    $description = [];
    $description[] = t('This action will delete the transaction as shown below, and cannot be undone.');
    
    $table = [
      'rows' => [],
      'header' => []
    ];
    
    foreach ($listview as $values) {
      $title = str_replace('_', ' ', str_replace('-', ' ', $values));
      $title = $title == 'time' ? 'date' : $title;
      $title = ucwords(strtolower($title));
      $table['header'][$values]['data'] = t($title);
      unset ($title);
      if (in_array($values, ['time', 'total']))
        $table['header'][$values]['style'] = 'text-align: right';
    }
    
    $configs = variable_get('mosquefinance_configs', []);
    $cash = dncfilewrapper_get_data('accounts', $configs['accounts']['cash']);
    foreach ($form_state['storage']['items'] as $keys => $values) {
      $data = dncfilewrapper_get_data('proposals', $values);
      if (empty($data['nid'])) {
        unset ($data);
        continue;
      }
      
      $journals = [$data['journals'][0]['account'], $data['journals'][1]['account']];
      unset ($journals);
      
      foreach ($listview as $value) {
        if ($value == 'time') {
          $data[$value] = dncsystem_date('j-M-Y', $data[$value]);
        }
        elseif ($value == 'total') {
          $data[$value] = 'Rp. ' . number_format($data[$value], 0, ',', '.') . ',-';
        }
        if ($structure[$value]['type'] == 'reference' && !empty($data[$value])) {
          $ref = dncfilewrapper_get_data($structure[$value]['reference'], $data[$value]);
          $data[$value] = empty($ref['title']) ? NULL : $ref['title'];
          unset ($ref);
        }
        $table['rows'][$keys][$value]['data'] = empty($data[$value]) ? NULL : $data[$value];
        if (in_array($value, ['time', 'total']))
          $table['rows'][$keys][$value]['style'] = 'text-align: right';
      }
      $data['action'] = empty($data['action']) ? NULL : ucwords(strtolower($data['action']));
      $table['rows'][$keys]['title']['data'] = (empty($data['action']) ? NULL : '[' . t($data['action']) . '] ') . $table['rows'][$keys]['title']['data'];
      unset ($data);
    }
    unset ($cash, $configs);
    $description[] = theme('table', $table);
    unset ($table);
    
    $description[] = '<br />';
    $description = implode("\n", $description);
    $yes = t('Yes Save Now');
    $no = t('Cancel');
    
    unset ($firstbalance, $endbalance);
    return confirm_form($form, $question, $path, $description, $yes, $no);
  }

  $count = dncfilewrapper_get_listing('proposals', [], [], TRUE);
  if (empty($count)) {
    unset ($structure, $listview, $count);
    drupal_set_message(t('No data available yet'), 'error', FALSE);
    return $form;
  }
  
  $form['first'] = [
    '#type' => 'item',
    '#markup' => '<strong>' . t('First Balance') . (empty($firstbalance['date']) ? NULL : ' ' . t('per @date', ['@date' => dncsystem_date('j F Y', strtotime($firstbalance['date']))])) . ':</strong> Rp. ' . number_format($firstbalance['balance'], 0, ',', '.') . ',-',
    '#weight' => 5
  ];
  
  $form['items'] = [
    '#type' => 'tableselect',
    '#weight' => 10,
    '#options' => [],
    '#header' => []
  ];
  foreach ($listview as $values) {
    $title = str_replace('_', ' ', str_replace('-', ' ', $values));
    $title = $title == 'time' ? 'date' : $title;
    $title = ucwords(strtolower($title));
    $form['items']['#header'][$values]['data'] = t($title);
    unset ($title);
    if (in_array($values, ['time', 'total'])) {
      $form['items']['#header'][$values]['style'] = 'text-align: right';
    }
  }
  
  $limit = 50;
  $current = pager_default_initialize($count, $limit);
  $current = $current * $limit;
  $data = dncfilewrapper_get_listing('proposals', [], [$current, $limit]);
  unset ($current, $limit);
  
  $configs = variable_get('mosquefinance_configs', []);
  $cash = dncfilewrapper_get_data('accounts', $configs['accounts']['cash']);
  foreach ($data as $keys => $values) {
    $journals = [];
    foreach ($values['journals'] as $value) {
      $journals[$value['action']] = $value['account'];
      if ($value['account'] == $configs['accounts']['cash']) {
        if ($value['action'] != $cash['normal']) {
          $endbalance -= $value['worth'];
        }
        else {
          $endbalance += $value['worth'];
        }
      }
    }
    unset ($journals);
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
      
      if ($value == 'time') {
        $values[$value] = dncsystem_date('j-M-Y', $values[$value]);
      }
      
      $form['items']['#options'][$values['nid']][$value]['data'] = $value == 'time' ? l($values[$value], 'mosquefinance/action/recents/' . $values['nid']) : (empty($values[$value]) ? NULL : (is_numeric($values[$value]) ? number_format($values[$value], 0, ',', '.') : $values[$value]));
      if (in_array($value, ['time', 'total'])) {
        $form['items']['#options'][$values['nid']][$value]['style'] = 'text-align: right';
      }
      elseif ($value == 'title') {
        $values['action'] = ucwords(strtolower($values['action']));
        $form['items']['#options'][$values['nid']][$value]['data'] = (empty($values['action']) ? NULL : '[' . t($values['action']) . '] ') . $form['items']['#options'][$values['nid']][$value]['data'];
      }
    }
  }
  unset ($structure, $listview, $scenarios, $configs, $cash, $firstbalance);
  
  $form['end'] = [
    '#type' => 'item',
    '#markup' => '<strong>' . t('End Balance') . ':</strong> Rp. ' . number_format($endbalance, 0, ',', '.') . ',-',
    '#weight' => 20
  ];
  unset ($endbalance);
  
  $form['submit'] = [
    '#type' => 'submit',
    '#weight' => 100,
    '#value' => t('Delete choosed')
  ];
  $form['pager'] = [
    '#type' => 'item',
    '#weight' => 200,
    '#markup' => theme('pager', ['quantity' => 10])
  ];
  
  if (user_access('administer mosque finance')) {
    $form['validation'] = [
      '#type' => 'fieldset',
      '#title' => t('Validation'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 500
    ];
    $message = [];
    $message[] = '<h4><strong>' . t('Validate all these transactions above. You should check and validate the correctness of the transaction. Without your validation and approval, the transaction will not be accepted by the system and is not recorded in the weekly and monthly reports.') . '</strong></h4>';
    $form['validation']['infomessage'] = [
      '#type' => 'item',
      '#weight' => 3,
      '#markup' => implode("\n", $message),
    ];
    unset ($message);
      
    $balance = dncfilewrapper_get_listing('balances');
    if (!empty($balance)) {
      $message = '<h3><strong style="color: red;">' . t('Just make sure that the above transaction data is correct so you are sure to do this validation. Remember once you execute this action, it can not be undone') . '</strong></h3>';
      $form['validation']['warnmessage'] = [
        '#type' => 'item',
        '#weight' => 5,
        '#markup' => $message,
      ];
      unset ($message);
      $form['validation']['validexec'] = [
        '#type' => 'submit',
        '#value' => t('Approve and validate'),
        '#weight' => 10,
        '#submit' => ['mosquefinance_action_newly_validation'],
        '#attributes' => [
          'class' => ['btn', 'btn-warning']
        ]
      ];
    }
    else {
      $form['validation']['vmessage'] = [
        '#type' => 'item',
        '#weight' => 10,
        '#markup' => '<h4><strong style="color: red">' . t('Ups! To validate the recent transactions, fill the beginning balance first') . ' ' . l(t('here'), 'mosquefinance/action/beginning-balance') . '</strong></h4>'
      ];
    }
  }
  
  return $form;
}

function mosquefinance_action_recents_validate(&$form, &$form_state) {
  if (empty($form_state['values']['confirm'])) {
    if ($form_state['values']['op'] == t('Approve and validate')) {
      return;
    }
//    echo '<pre>'; print_r($form_state['values']); echo '</pre>'; exit;
    $form_state['storage']['confirm'] = TRUE;
    $form_state['rebuild'] = TRUE;
    foreach ($form_state['input']['items'] as $keys => $values) {
      if (!empty($values)) {
        $form_state['storage']['items'][] = $keys;
      }
    }
    if (empty($form_state['storage']['items'])) {
      unset ($form_state['storage']);
      form_set_error('items', t('No items found to be deleted'));
      return;
    }
  }
}

function mosquefinance_action_recents_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['redirect'] = current_path();
  if (empty($form_state['values']['confirm'])) {
    return;
  }
  $data = [];
  foreach ($form_state['values']['items'] as $values) {
    $data[] = ['nid' => $values];
  }
  $result = dncfilewrapper_delete_data($GLOBALS['user']->uid, ['proposals' => $data]);
  unset ($data);
  if (empty($result)) {
    unset ($result);
    drupal_set_message(t('Failed to delete these items'), 'error', TRUE);
    return;
  }
  unset ($result);
  drupal_set_message(t('Success to delete these items'));
}

function mosquefinance_action_recents_edit(array $form = [], array $form_state = [], $nid) {
  if (!empty($form_state['storage']['confirm'])) {
    foreach ($form_state['storage'] as $keys => $values) {
      $form[$keys] = [
        '#type' => 'value',
        '#value' => $values
      ];
    }
    $question = t('Are you sure to save the transaction') . ' ?';
    $path = 'mosquefinance/action/recents';
    $description = [];
    $description[] = t('This action will save the transaction as shown below. Therefore check back and make sure whether the items in the transaction are in accordance with what you mean.');
    $description[] = '<ul>';
    
    $structure = mosquefinance_structures_of_data('proposals');
    foreach ($structure as $keys => $values) {
      if ($keys == 'journals') {
        continue;
      }
      $title = str_replace('_', ' ', str_replace('-', ' ', $keys));
      $title = $title == 'time' ? 'date' : $title;
      $title = ucwords(strtolower($title));
      $data = $keys == 'time' ? dncsystem_date('j F Y' , $form_state['storage'][$keys]) : $form_state['storage'][$keys];
      if ($values['type'] == 'reference') {
        $reference = dncfilewrapper_get_data($values['reference'], $data);
        $data = empty($reference['title']) ? NULL : $reference['title'];
        unset ($reference);
      }
      elseif ($keys == 'total') {
        $data = 'Rp. ' . number_format($data, 0, ',', '.') . ',-';
      }
      elseif ($keys == 'action') {
        $options = [
          'incoming' => t('Incoming'),
          'expending' => t('Expending'),
          'crediting' => t('Crediting'),
          'debting' => t('Debting')
        ];
        $data = $options[$data];
        unset ($options);
      }
      $description[] = '<li><strong>' . t($title) . ':</strong> ' . $data . '</li>';
      unset ($title);
      unset ($data);
    }
    unset ($structure);
    
    $description[] = '</ul>';
    $description[] = '<br />';
    $description = implode("\n", $description);
    $yes = t('Yes Save Now');
    $no = t('Cancel');
    return confirm_form($form, $question, $path, $description, $yes, $no);
  }
  
  $data = dncfilewrapper_get_data('proposals', $nid);
  if (empty($data['nid'])) {
    unset ($data);
    drupal_set_message(t('No transaction found by the ID'), 'error', FALSE);
    return $form;
  }
  $form['nid'] = [
    '#type' => 'value',
    '#value' => $nid
  ];
  $structure = mosquefinance_structures_of_data('proposals');
  $i = 1;
  foreach ($structure as $keys => $values) {
    if ($keys == 'journals') {
      continue;
    }
    $title = str_replace('_', ' ', str_replace('-', ' ', $keys));
    $title = $title == 'time' ? 'date' : $title;
    $title = ucwords(strtolower($title));
    $form[$keys] = [
      '#title' => t($title),
      '#default_value' => empty($data[$keys]) ? NULL : $data[$keys],
      '#weight' => $i * 10
    ];
    $i++;
    if ($values['type'] == 'reference') {
      $form[$keys]['#type'] = 'select';
      $options = ['_none' => '-- ' . t('Choose one below') . ' --'];
      $dump = dncfilewrapper_get_listing($values['reference']);
      if (!empty($dump)) {
        foreach ($dump as $value) {
          $options[$value['nid']] = $value['title'];
        }
      }
      $form[$keys]['#options'] = $options;
      unset ($options, $dump);
    }
    else {
      $form[$keys]['#type'] = 'textfield';
      if ($keys == 'time') {
        if (module_exists('date_popup')) {
          $form[$keys]['#type'] = 'date_popup';
          $form[$keys]['#date_format'] = 'Y-m-d';
          $form[$keys]['#date_label_position'] = 'none';
          $form[$keys]['#date_year_range'] = '-1:0';
          $form[$keys]['#default_value'] = empty($data[$keys]) ? NULL : date('Y-m-d H:i:s', $data[$keys]);
          $form['spacer'] = [
            '#type' => 'item',
            '#markup' => '',
            '#weight' => $form[$keys]['#weight'] + 0.1,
          ];
        }
        else {
          $form[$keys]['#size'] = 10;
          $form[$keys]['#maxlength'] = 10;
          $form[$keys]['#attributes']['placeholder'] = $title;
        }
      }
      elseif($keys == 'action') {
        $form[$keys]['#type'] = 'value';
        $form[$keys]['#value'] = $form[$keys]['#default_value'];
        $options = [
          'incoming' => t('Incoming'),
          'expending' => t('Expending'),
          'crediting' => t('Crediting'),
          'debting' => t('Debting')
        ];
        $form['v_' . $keys] = [
          '#type' => 'item',
          '#title' => $form[$keys]['#title'],
          '#weight' => $form[$keys]['#weight'],
          '#markup' => $options[$form[$keys]['#value']]
        ];
        unset ($form[$keys]['#default_value']);
        unset ($form[$keys]['#weight']);
        unset ($options);
      }
    }
    if (!empty($values['required'])) {
      $form[$keys]['#required'] = TRUE;
    }
    unset ($title);
  }
  unset ($data, $structure, $i);
  $form['submit'] = [
    '#type' => 'submit',
    '#weight' => 100,
    '#value' => t('Save')
  ];
  return $form;
}

function mosquefinance_action_recents_edit_validate(&$form, &$form_state) {
  if (empty($form_state['values']['confirm'])) {
    $form_state['rebuild'] = TRUE;
    $form_state['storage']['confirm'] = TRUE;
    $form_state['storage']['nid'] = $form_state['values']['nid'];
    $structure = mosquefinance_structures_of_data('proposals');
    foreach ($structure as $keys => $values) {
      if ($keys == 'journals') {
        continue;
      }
      if (!empty($form_state['input'][$keys])) {
        if (!empty($form_state['input'][$keys]['date'])) {
          $form_state['input'][$keys] = $form_state['input'][$keys]['date'];
        }
        if ($keys == 'time') {
          $form_state['input'][$keys] = strtotime($form_state['input'][$keys]);
        }
        $form_state['storage'][$keys] = $form_state['input'][$keys];
      }
      else {
        if (!empty($form_state['values'][$keys])) {
          $form_state['storage'][$keys] = $form_state['values'][$keys];
        }
      }
    }
    unset ($structure);
  }
}

function mosquefinance_action_recents_edit_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['redirect'] = 'mosquefinance/action/recents';
  if (empty($form_state['values']['nid']) || empty($form_state['values']['total'])) {
    drupal_set_message(t('No ID information or empty total'), 'error', FALSE);
    return;
  }
  $data = dncfilewrapper_get_data('proposals', $form_state['values']['nid']);
  if (empty($data['nid'])) {
    unset ($data);
    drupal_set_message(t('No transaction found by the ID'), 'error', FALSE);
    return;
  }
  foreach ($data as $keys => $values) {
    if ($keys == 'journals') {
      foreach ($values as $key => $value) {
        $data[$keys][$key]['worth'] = $form_state['values']['total'];
        settype($data[$keys][$key]['worth'], 'int');
      }
    }
    else {
      if (!empty($form_state['values'][$keys])) {
        $data[$keys] = $form_state['values'][$keys];
      }
    }
  }
  $data = mosquefinance_set_data_type('proposals', $data);
  $result = dncfilewrapper_update_data($GLOBALS['user']->uid, ['proposals' => [$data]]);
  unset ($data);
  if (empty($result['proposals'][0]['nid'])) {
    unset ($result);
    drupal_set_message(t('Failed to save transaction'), 'error', FALSE);
    return;
  }
  unset ($result);
  drupal_set_message(t('Successfully save the transaction'));
}

function mosquefinance_action_newly_validation(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['redirect'] = current_path();
  $pdata = dncfilewrapper_get_listing('proposals');
  if (empty($pdata)) {
    unset ($pdata);
    drupal_set_message(t('No newly transactions found to post to accounting management'), 'error', FALSE);
    return;
  }
  $tdata = [];
  $jdata = [];
  $balance = [];
  $balance[0] = mosquefinance_get_balance();
  $balance[0] = empty($balance[0]['balance']) ? 0 : $balance[0]['balance'];
  settype($balance[0], 'int');
  $balance[1] = $balance[0];
  
  $configs = variable_get('mosquefinance_configs', []);
  $cash = dncfilewrapper_get_data('accounts', $configs['accounts']['cash']);
  
  $structure = dnctinyaccounting_structures_of_data();
  foreach ($pdata as $keys => $values) {
    foreach ($structure['transactions'] as $key => $value) {
      if (isset($values[$key])) {
        $tdata[$keys][$key] = $values[$key];
      }
      else {
        if (!empty($value['required'])) {
          unset ($tdata[$keys]);
          break;
        }
      }
    }
    if (!empty($tdata[$keys])) {
      $tdata[$keys]['proposal'] = $values['nid'];
      foreach ($values['journals'] as $key => $value) {
        $account = dncfilewrapper_get_data('accounts', $value['account']);
        $journal = [
          'proposal' => $keys,
          'title' => $account['title'],
          'time' => $values['time']
        ];
        foreach($structure['journals'] as $ke => $val) {
          if (isset($value[$ke])) {
            $journal[$ke] = !empty($journal[$ke]) ? $journal[$ke] : $value[$ke];
          }
          else {
            if (!empty($val['required']) && empty($journal[$ke])) {
              if ($ke != 'transaction') {
                unset ($journal);
                break;
              }
            }
          }
        }
        if (!empty($journal)) {
          $jdata[] = $journal;
          if ($journal['account'] == $cash['nid']) {
            if ($journal['action'] != $cash['normal']) {
              $balance[1] -= $journal['worth'];
            }
            else {
              $balance[1] += $journal['worth'];
            }
          }
        }
        unset ($journal);
      }
    }
  }
  unset ($pdata, $configs, $cash);
  settype($balance[1], 'int');
  $balance = [
    'date' => mosquefinance_get_frydate_from_now(),
    'balance' => $balance[1]
  ];
//  echo '<pre>'; print_r([$balance, $tdata, $jdata]); echo '</pre>'; exit;
  
  $datasource = [
    'transactions' => $tdata,
    'journals' => $jdata,
    'balance' => $balance
  ];
  unset ($tdata, $jdata, $balance);
  
  $operations = [];
  // save transactions
  $operations[] = ['mosquefinance_save_transactions', [$datasource]];
  // update proposals and remove
  $operations[] = ['mosquefinance_update_proposals', [$datasource]];
  // save journals
  $operations[] = ['mosquefinance_save_journals', [$datasource]];
  // save balance
  $operations[] = ['mosquefinance_update_balance', [$datasource]];
  
  $batch_definition = [
    'operations' => $operations,
    'title' => t('Validating recent transactions'),
    'init_message' => t('Progress start...'),
    'progress_message' => t('Precessed @current out of @total'),
    'error_message' => t('Process has encountered an error'),
    'finished' => 'mosquefinance_saving_recent_finished'
  ];
  unset ($operations);
  batch_set($batch_definition);
}