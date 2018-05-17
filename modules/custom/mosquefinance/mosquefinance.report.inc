<?php

function mosquefinance_report_menu(&$items) {
  $items['mosquefinance/report/weekly'] = [
    'title' => 'Weekly Report',
    'description' => 'Finance weekly report',
    'access callback' => TRUE,
    'page callback' => 'mosquefinance_report_weekly',
    'file' => 'mosquefinance.report.inc',
    'type' => MENU_CALLBACK
  ];
  $items['mosquefinance/report-form/weekly'] = [
    'title' => 'Weekly Report',
    'description' => 'Finance weekly report',
    'access callback' => TRUE,
    'page callback' => 'drupal_get_form',
    'page arguments' => ['mosquefinance_report_weekly_form'],
    'file' => 'mosquefinance.report.inc',
  ];
}

function mosquefinance_report_weekly() {
  $date = empty($_REQUEST['date']) ? NULL : $_REQUEST['date'];
  if (!empty($date)) {
    $friday = mosquefinance_get_frydate_from_now();
    if (strtotime($date) > $friday) {
      drupal_set_message(t('Next friday is @day. Your requested date is too far than the next friday.', [
        '@day' => dncsystem_date('j F Y', $friday)
      ]), 'error', FALSE);
      unset ($friday, $date);
      return '';
    }
    unset ($friday);
  }
  $result = mosquefinance_get_balance($date);
  unset ($date);
  if (empty($result)) {
    drupal_set_message(t('No transactions have occurred or been approved.'), 'error', FALSE);
    return '';
  }
  
  $configs = variable_get('mosquefinance_configs', []);
  $acash = dncfilewrapper_get_data('accounts', $configs['accounts']['cash']);
  
  $output = [];
  // get the date balance
  $transactions = [];
  $balance = 0;
  if ($result['nid'] < 2) {
    $cashflows = mosquefinance_cashflow_time_indexed();
    if (!empty($cashflows)) {
      foreach ($cashflows as $values) {
        if (strtotime($values) > strtotime($result['date']))
          continue;
        $dump = mosquefinance_cashflow_time_indexed($values);
        if (empty($dump)) {
          unset($dump);
          continue;
        }
        foreach ($dump as $value) {
          $transactions[] = $value;
        }
        unset ($dump);
      }
    }
    unset ($cashflows);
  }
  else {
    $dateend = $result['date'];
    $datestart = $dateend;
    for ($i = ($result['nid'] - 1); $i > 0; $i--) {
      $bdata = dncfilewrapper_get_data('balances', $i);
      $balance = $bdata['balance'];
      $datestart = $bdata['date'];
      unset ($bdata);
      break;
    }
    $datestart = strtotime($datestart);
    $dateend = strtotime($dateend);
    for ($i = $datestart; $i <= $dateend; $i += (60 * 60 * 24)) {
      $idate = date('Y-m-d', $i);
      $trindex = mosquefinance_cashflow_time_indexed($idate);
      if (empty($trindex)) {
        unset ($idate, $trindex);
        continue;
      }
      foreach ($trindex as $keys => $values) {
        $transactions[] = $values;
      }
      unset ($idate, $trindex);
    }
    unset ($dateend, $datestart);
//    echo '<pre>'; print_r($transactions); echo '</pre>'; exit;
  }

  $output[] = '<div align="center">';
  $output[] = '<h3><strong>' . t('Finance Report of @site', ['@site' => ucwords(variable_get('site_name', 'drupal'))]) . '</strong></h3>';
  $output[] = '<h4><strong>' . t('per Friday') . ', ' . mosquefinance_tohijr($result['date']) . ' / ' . dncsystem_date('j F Y', strtotime($result['date'])) . '</strong></h4>';
  $output[] = '</div>';
  
  $table = [
    'rows' => [],
    'header' => [
      'num' => ['data' => ''],
      'title' => ['data' => ''],
      'worth' => ['data' => ''],
      'total' => ['data' => '']
    ]
  ];
  
  $table['rows'][] = [
    'num' => [
      'data' => '<h4><strong>' . t('Starting Balance on previous friday') . '</strong></h4>',
      'colspan' => 3
    ],
    'total' => [
      'data' => '<h4>' . number_format($balance, 0, ',', '.') . '</h4>',
      'style' => 'text-align: right'
    ]
  ];
  
  $table['rows'][] = [
    'num' => [
      'data' => '<h4><strong>' . t('Incoming') . '</strong></h4>',
      'colspan' => 4
    ]
  ];
  
  $total = 0;
  foreach ($transactions as $keys => $values) {
    if (empty($values['normal_action'])) {
      $data = dncfilewrapper_get_data('transactions', $values['transaction']);
      $table['rows'][] = [
        'num' => [
          'data' => '-',
          'style' => 'text-align: right'
        ],
        'title' => ['data' => $data['title']],
        'worth' => [
          'data' => number_format($values['worth'], 0, ',', '.'),
          'style' => 'text-align: right'
        ],
        'total' => [
          'data' => '',
          'style' => 'text-align: right'
        ]
      ];
      unset ($data);
      $total += $values['worth'];
    }
  }
  
  $table['rows'][] = [
    'num' => [
      'data' => '',
    ],
    'title' => [
      'data' => '<h4>' . t('Total Incoming') . '</h4>',
      'colspan' => 2,
    ],
    'total' => [
      'data' => '<h4>' . number_format($total, 0, ',', '.') . '</h4>',
      'style' => 'text-align: right'
    ]
  ];
  
//  $table['rows'][] = [
//    'num' => [
//      'data' => '',
//      'colspan' => 3
//    ],
//    'total' => [
//      'data' => '<h4>' . number_format(($balance + $total), 0, ',', '.') . '</h4>',
//      'style' => 'text-align: right'
//    ]
//  ];
  
  $table['rows'][] = [
    'num' => [
      'data' => '<h4><strong>' . t('Expending') . '</strong></h4>',
      'colspan' => 4
    ]
  ];
  
  $total = 0;
  foreach ($transactions as $keys => $values) {
    if (!empty($values['normal_action'])) {
      $data = dncfilewrapper_get_data('transactions', $values['transaction']);
      $table['rows'][] = [
        'num' => [
          'data' => '-',
          'style' => 'text-align: right'
        ],
        'title' => ['data' => $data['title']],
        'worth' => [
          'data' => number_format($values['worth'], 0, ',', '.'),
          'style' => 'text-align: right'
        ],
        'total' => [
          'data' => '',
          'style' => 'text-align: right'
        ]
      ];
      unset ($data);
      $total += $values['worth'];
    }
  }
  
  $table['rows'][] = [
    'num' => [
      'data' => '',
    ],
    'title' => [
      'data' => '<h4>' . t('Total Expending') . '</h4>',
      'colspan' => 2,
    ],
    'total' => [
      'data' => '<h4>' . number_format($total, 0, ',', '.') . '</h4>',
      'style' => 'text-align: right'
    ]
  ];
  
  $table['rows'][] = [
    'num' => [
      'data' => '<h3><strong>' . t('Ending Balance') . '</strong></h3>',
      'colspan' => 3
    ],
    'total' => [
      'data' => '<h3><strong>' . number_format($result['balance'], 0, ',', '.') . '</strong></h3>',
      'style' => 'text-align: right'
    ]
  ];
  unset ($total);
  
  $output[] = theme('table', $table);
  unset ($table);
  
  unset ($configs, $acash, $balance, $transactions, $result);
  
  $output = implode("\n", $output);
  return $output;
}

function mosquefinance_report_weekly_form(array $form = [], array $form_state = []) {
  $form['date'] = [
    '#title' => t('Choose the date'),
    '#description' => t('Please select a date on Friday'),
    '#weight' => 10
  ];
  
  if (module_exists('date_popup')) {
    $form['date']['#type'] = 'date_popup';
    $form['date']['#date_format'] = 'Y-m-d';
    $form['date']['#date_type'] = DATE_FORMAT_DATE;
    $form['date']['#date_year_range'] = '-1:0';
    $form['date']['#date_label_position'] = 'none';
  }
  else {
    $form['date']['#type'] = 'textfield';
    $form['date']['#size'] = 10;
    $form['date']['#maxlength'] = 10;
    $form['date']['#description'] .= ' ' . t('Please use UNIX standard format YYYY-MM-DD, e.g. @datenow', [
      '@datenow' => date('Y-m-d', time())
    ]);
  }
  
  $form['spacer'] = [
    '#type' => 'item',
    '#weight' => $form['date']['#weight'] + 0.1,
    '#markup' => ''
  ];
  
  $form['submit'] = [
    '#type' => 'submit',
    '#weight' => 100,
    '#value' => t('Show'),
    '#attributes' => [
      'class' => ['btn', 'btn-primary']
    ]
  ];
  return $form;
}

function mosquefinance_report_weekly_form_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['input']['date'] = empty($form_state['input']['date']['date']) ? $form_state['input']['date'] : $form_state['input']['date']['date'];
  $w = date('w', strtotime($form_state['input']['date']));
  if ($w != 5) {
    unset ($w);
    form_set_error('date', t('The selected date must be Friday'));
    return;
  }
  unset ($w);
  
  $form_state['input']['date'] = date('Y-m-d', strtotime($form_state['input']['date']));
  
  $data = mosquefinance_get_balance($form_state['input']['date']);
  if (empty($data)) {
    unset ($data);
    form_set_error('date', t('Nothing on that date'));
    return;
  }
  unset ($data);
  
  $form_state['redirect'] = ['mosquefinance/report/weekly', [
    'query' => [
      'date' => $form_state['input']['date']
    ]
  ]];
}