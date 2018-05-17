<?php
/**
 * @file
 * Enables modules and site configuration for a standard site installation.
 */

/**
 * Implements hook_form_FORM_ID_alter() for install_configure_form().
 *
 * Allows the profile to alter the site configuration form.
 */
function mosque_form_install_configure_form_alter(&$form, &$form_state) {
  // Pre-populate the site name with the server name.
  $form['site_information']['site_name']['#default_value'] = $_SERVER['SERVER_NAME'];
}

function mosque_install_tasks_alter(&$tasks) {
  unset ($tasks['install_configure_form']);
  $target_theme = 'seven';
  if ($GLOBALS['theme'] != $target_theme) {
    unset($GLOBALS['theme']);

    drupal_static_reset();
    $GLOBALS['conf']['maintenance_theme'] = $target_theme;
    _drupal_maintenance_theme();
  }
}

function mosque_nomenclatures() {
  return [
    'site_name' => 'Masjid Nurul Hikmah',
    'site_mail' => 'site@examle.org',
    'date_default_time_zone' => 'Asia/Makassar',
    'site_default_country' => 'ID',
    'clean_url' => 0,
    'install_time' => $_SERVER['REQUEST_TIME'],
  ];
}

function mosque_resources() {
  return [
    [
      'name' => 'fancy_login_login_block',
      'region' => 'navigation'
//    ],
//    [
//      'name' => 'dncaccountqueueaction',
//      'region' => 'sidebar_second',
//      'weight' => 0,
//    ],
//    [
//      'name' => 'dncaccountingbalances',
//      'region' => 'sidebar_second',
//      'weight' => 1
    ]
  ];
}

function mosque_prepare_main_menus() {
  return [
    'parent' => [
      'reference' => [
        'link_title' => st('Reference'),
        'link_path' => '<front>',
        'menu_name' => 'main-menu',
        'expanded' => TRUE,
        'weight' => 1
      ],
      'transaction' => [
        'link_title' => st('Transaction'),
        'link_path' => '<front>',
        'menu_name' => 'main-menu',
        'expanded' => TRUE,
        'weight' => 3
      ],
      'report' => [
        'link_title' => st('Report'),
        'link_path' => '<front>',
        'menu_name' => 'main-menu',
        'expanded' => TRUE,
        'weight' => 4
      ]
    ],
    'children' => [
      'account' => [
        'link_title' => st('Account'),
        'link_path' => 'dnctinyaccounting/references/accounts',
        'menu_name' => 'main-menu',
        'weight' => 11,
        'parent' => 'reference'
      ],
      'balance' => [
        'link_title' => st('Beginning Balance'),
        'link_path' => 'mosquefinance/action/beginning-balance',
        'menu_name' => 'main-menu',
        'weight' => 12,
        'parent' => 'reference'
      ],
      'pic' => [
        'link_title' => st('Person In Charge'),
        'link_path' => 'mosquefinance/references/entities',
        'menu_name' => 'main-menu',
        'weight' => 13,
        'parent' => 'reference'
      ],
      'incoming' => [
        'link_title' => st('Incoming'),
        'link_path' => 'mosquefinance/action/incoming',
        'menu_name' => 'main-menu',
        'weight' => 31,
        'parent' => 'transaction'
      ],
      'expending' => [
        'link_title' => st('Expending'),
        'link_path' => 'mosquefinance/action/expending',
        'menu_name' => 'main-menu',
        'weight' => 32,
        'parent' => 'transaction'
      ],
      'debting' => [
        'link_title' => st('Debting'),
        'link_path' => 'mosquefinance/action/debting',
        'menu_name' => 'main-menu',
        'weight' => 33,
        'parent' => 'transaction'
      ],
      'crediting' => [
        'link_title' => st('Claiming'),
        'link_path' => 'mosquefinance/action/crediting',
        'menu_name' => 'main-menu',
        'weight' => 34,
        'parent' => 'transaction'
      ],
      'recently' => [
        'link_title' => st('Recents'),
        'link_path' => 'mosquefinance/action/recents',
        'menu_name' => 'main-menu',
        'weight' => 35,
        'parent' => 'transaction'
      ],
      'weekly' => [
        'link_title' => st('Weekly'),
        'link_path' => 'mosquefinance/report-form/weekly',
        'menu_name' => 'main-menu',
        'weight' => 41,
        'parent' => 'report'
      ]
    ]
  ];
}