<?php

/**
 * @file
 * All of the function to manipulate content
 * in custom dnc system ways
 */

/**
 * Listing of registered content type in the system
 *
 * @param string $module
 *   module info
 * @param array $scheme
 *   scheme info
 * @param string $detailpath
 *   path of detail info
 *
 * @return string
 *   listing available content types in html list
 */
function dncsystem_listing_content_type($module, $scheme, $detailpath) {
  if (empty($scheme)) {
    return;
  }
  $output = '<ul>';
  $masters = array();
  foreach ($scheme as $key => $value) {
    if (!empty($value['content_type'])) {
      $masters[$key] = node_type_get_type($value['content_type']);
    }
  }
  unset ($scheme);
  foreach ($masters as $key => $value) {
    $output .= '<li>' . l($value->name, $detailpath . '/' . $key) . '<br />' . $value->description . '</li>';
  }
  unset ($masters);
  $output .= '</ul>';
  return $output;
}

/**
 * Extract encoded master request
 *
 * @param string $master
 *   base64 encoded request
 *
 * @return array
 *   master variable array result if successfull, NULL if otherwise
 */
function dncsystem_extract_encoded_master($master) {
  $master = empty($master) ? NULL : base64_decode(urldecode($master));
  if (empty($master)) {
    unset ($master);
    return;
  }
  $master = json_decode($master, TRUE);
  if (empty($master['module']) || empty($master['scheme']) || empty($master['master'])) {
    unset ($master);
    return;
  }
  return $master;
}

/**
 * Populating table select to operate managemen of master data
 *
 * @param array $form
 *   $form variable array
 * @param array $form_state
 *   $form_state variable array
 * @param boolean $titlesearch
 *   whether for searching node by title or not
 * @param string $extrafunction
 *   another function if any
 *
 * @return array
 *   table array variable to populate in the form
 */
function dncsystem_populate_tableselect(&$form, &$form_state, $titlesearch = FALSE, $extrafunction = NULL) {
  $return = array(
    'headers' => array(
      'num' => array(
        'data' => t('No.'),
        'type' => 'constant',
        'style' => 'text-align: right',
      ),
    ),
    'options' => array(),
  );
  $crows = 10;
  $scheme = $form_state['storage']['scheme'];
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node');
  $query->entityCondition('bundle', $scheme['content_type']);
  if ($titlesearch !== TRUE) {
    $query->propertyCondition('status', NODE_PUBLISHED);
  }
  else {
    $query->addTag('searching_dnc_master');
    $query->addMetaData('string', $form_state['storage']['title']);
    $query->addMetaData('scheme', $scheme);
  }
  $query->addMetaData('account', user_load(1));
  $nodetype = node_type_get_type($scheme['content_type']);
  foreach ($scheme['fields'] as $key => $value) {
    if ($value == 'title') {
      $return['headers'][$key]['data'] = $nodetype->title_label;
      $return['headers'][$key]['type'] = 'property';
      $return['headers'][$key]['sort'] = 'asc';
      $return['headers'][$key]['specifier'] = $value;
//      if ($key == 'title' && $titlesearch !== FALSE && !empty($form_state['storage']['title'])) {
//        $query->propertyCondition($value, '%' . $form_state['storage']['title'] . '%', 'LIKE');
//      }
    }
    else {
      $dump = field_info_instance('node', $value, $scheme['content_type']);
      $return['headers'][$key]['data'] = $dump['label'];
      $return['headers'][$key]['type'] = 'field';
      $dump = field_info_field($value);
      $return['headers'][$key]['specifier'] = array(
        'field' => $value,
        'column' => $dump['type'] == 'entityreference' ? 'target_id' : 'value',
      );
      unset ($dump);
//      if ($key == 'title' && $titlesearch !== FALSE && !empty($form_state['storage']['title'])) {
//        $query->fieldCondition($value, $return['headers']['title']['specifier']['column'], '%' . $form_state['storage']['title'] . '%', 'LIKE');
//      }
    }
  }
  unset ($nodetype);
  $count = $query->execute();
  $count = empty($count['node']) ? 0 : count($count['node']);
  $return['count'] = $count;

  $query->pager($crows);
  $query->tableSort($return['headers']);
  $result = $query->execute();
  unset ($query, $scheme);
  if (count($result) > 0 && !empty($result['node'])) {
    $page = empty($_REQUEST['page']) ? 0 : $_REQUEST['page'];
    $i = ($page * $crows) + 1;
    $nodes = node_load_multiple(array_keys($result['node']));
    foreach ($nodes as $node) {
      foreach ($return['headers'] as $key => $value) {
        $dump = NULL;
        if ($value['type'] == 'constant') {
          $dump = $i . '.';
          $i ++;
          $return['options'][$node->nid][$key]['style'] = 'text-align: right';
        }
        elseif ($value['type'] == 'property') {
          $dump = $key != 'title' ?
            $node->{$value['specifier']} : l($node->{$value['specifier']}, 'node/' . $node->nid . '/edit/', array(
              'query' => array(
                'destination' => 'admin/structure/dncsystem/' . $form_state['storage']['module'] . '/' . $form_state['storage']['master'],
              ),
            ));
        }
        else {
          $dmp = field_get_items('node', $node, $value['specifier']['field']);
          if(!empty($dmp) && count($dmp) > 0) {
            foreach ($dmp as $vl) {
              $dp = field_view_value('node', $node, $value['specifier']['field'], $vl, 'full');
              $dump[] = render($dp);
              unset ($dp);
            }
          }
          unset ($display, $dmp);
          $dump = !empty($dump) && count($dump) > 0 ? implode(', ', $dump) : NULL;
          $dump = $key != 'title' ? $dump : l($dump, 'node/' . $node->nid . '/edit/', array('query' => array('destination' => 'admin/structure/dncsystem/' . $form_state['storage']['module'] . '/' . $form_state['storage']['master'])));
        }
        $return['options'][$node->nid][$key]['data'] = $dump;
        $dump;
      }
    }
    unset ($nodes, $i);
  }
  unset($result, $scheme, $crows);
  if (!empty($extrafunction) && function_exists($extrafunction) && $titlesearch !== FALSE) {
    $return = $extrafunction($return, $form_state);
  }
  return $return;
}

/**
 * Confirmation to drop content
 *
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 * @param array $headers
 *   variable array of the headers if any
 *
 * @return array
 *   the confirmation form
 */
function dncsystem_drop_contents_confirmation(array $form = array(), array $form_state = array(), $headers) {
  $nodes = node_load_multiple($form_state['storage']['items']);
  $description = array();
  foreach ($nodes as $node) {
    if ($headers['title']['type'] == 'property') {
      $description[] =  $node->{$headers['title']['specifier']};
    }
    else {
      $dump = array();
      foreach ($node->{$headers['title']['specifier']['field']}[$node->language] as $value) {
        $dump[] = $value[$headers['specifier']['column']];
      }
      $description[] = '(' . implode(', ', $dump) . ')';
      unset ($dump);
    }
  }
  $nodetype = node_type_get_type($form_state['storage']['scheme']['content_type']);
  $description = implode(', ', $description);
  $description = t('This action will delete @master <strong>%item</strong> and cannot be undone!', array('@master' => $nodetype->name, '%item' => $description));
  unset ($nodes, $node);
  return confirm_form(
          $form,
          t('Are you sure to delete all these items?!'),
          current_path(),
          $description,
          t('Yes delete all now!'),
          t('cancel!')
          );
}

/**
 * Listing of the managed content
 *
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 * @param string $master
 *   content type of the data master if any
 *
 * @return array
 *   Array of the viewing form
 */
function dncsystem_manage_contents(array $form = array(), array $form_state = array(), $master = NULL) {
  if (empty($form_state['storage'])) {
    $master = dncsystem_extract_encoded_master($master);
    if (empty($master)) {
      return;
    }
    $form_state['storage']['module'] = $master['module'];
    $form_state['storage']['scheme'] = $master['scheme'];
    $form_state['storage']['master'] = $master['master'];
  }
  foreach ($form_state['storage'] as $key => $value) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => $value,
    );
    if ($key == 'scheme') {
      ${$key} = $value;
    }
  }
  $tableselect = dncsystem_populate_tableselect($form, $form_state);
  $nodetype = node_type_get_type($scheme['content_type']);
  $form['thetitle'] = array(
    '#markup' => t('<strong>@title</strong> has @count records',
              array(
                '@title' => $nodetype->name,
                '@count' => number_format($tableselect['count'], 0, ',', '.')
              )
            ) .
            (empty($tableselect['count']) ?
              NULL :
              ', ' . t('you can search by title') . ' ' .
              l(t('here'), 'admin/structure/dncsystem/dnccoass/' . $form_state['storage']['master'] . '/add')
            ),
    '#weight' => 0,
  );
  if (!empty($form_state['storage']['confirmed'])) {
    return dncsystem_drop_contents_confirmation($form, $form_state, $tableselect['headers']);
  }
  $form['items'] = array(
    '#type' => 'tableselect',
    '#empty' => t('No content available yet'),
    '#weight' => 1,
  );
  if (count($tableselect['options']) > 0) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete choosen items'),
      '#weight' => 10,
    );
  }
  $form['items']['#options'] = $tableselect['options'];
  $form['items']['#header'] = $tableselect['headers'];
  $form['pager'] = array(
    '#markup' => theme('pager', array('tags' => NULL)),
    '#weight' => 99,
  );
  unset ($tableselect);
  return $form;
}

/**
 * Submitting function of form data master management
 *
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 *
 * @return string
 *   message successfull if success, or massage failure or error otherwise
 */
function dncsystem_manage_contents_submit($form, &$form_state) {
  if (empty($form_state['values']['confirmed'])) {
    $form_state['storage']['confirmed'] = TRUE;
    foreach ($form_state['values'] as $key => $value) {
      if ($key == 'items') {
        $dump = array();
        foreach ($value as $vl) {
          if (!empty($vl)) {
            $dump[] = $vl;
          }
        }
        $value = $dump;
        unset ($dump);
      }
      $form_state['storage'][$key] = $value;
    }
    $form_state['rebuild'] = TRUE;
    return;
  }
  form_state_values_clean($form_state);
  foreach ($form_state['values']['items'] as $value) {
    $node = node_load($value);
    if (!empty($node) && is_object($node) && !empty($node->type)) {
      $node->status = NODE_NOT_PUBLISHED;
      node_save($node);
    }
    unset ($node);
  }
  drupal_set_message(t('All choosed items have been delete successfully!'));
  $form_state['redirect'] = current_path();
}

/**
 * Custom alternative form to add contents
 *
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 * @param string $master
 *   content type of the data master if any
 * @param string $extrafunction
 *   extra function name if any
 *
 * @return array
 *   form array of the operation
 */
function dncsystem_add_contents(array $form = array(), array $form_state = array(), $master = NULL, $extrafunction = NULL) {
  if (empty($form_state['storage'])) {
    $master = dncsystem_extract_encoded_master($master);
    if (empty($master)) {
      return;
    }
    $form_state['storage']['module'] = $master['module'];
    $form_state['storage']['scheme'] = $master['scheme'];
    $form_state['storage']['master'] = $master['master'];
    $form_state['storage']['destination'] = $master['destination'];
  }
  foreach ($form_state['storage'] as $key => $value) {
    $form[$key] = array(
      '#type' => 'value',
      '#value' => $value,
    );
    ${$key} = $value;
  }
  if (empty($form_state['storage']['title'])) {
    $form['title'] = array(
      '#type' => 'textfield',
      '#description' => t('Title of the master'),
      '#title' => t('Title'),
      '#size' => 40,
      '#required' => TRUE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Search'),
    );
    return $form;
  }

  $tableselect = dncsystem_populate_tableselect($form, $form_state, TRUE, $extrafunction);
  $nodetype = node_type_get_type($scheme['content_type']);
  $form['thetitle'] = array(
    '#markup' => $nodetype->name,
    '#weight' => 0,
  );
  $form['nid'] = array(
    '#type' => 'tableselect',
    '#empty' => t('No content available yet'),
    '#multiple' => FALSE,
    '#weight' => 1,
  );

  if (count($tableselect['options']) < 1) {
    drupal_goto('node/add/' . str_replace('_', '-', str_replace(' ', '-', $scheme['content_type'])), array(
      'query' => array(
        'destination' => $destination,
        'edit[title]' => $form_state['storage']['title'],
      ),
    ));
    return;
  }
  if (empty($tableselect['hidenewoption'])) {
    foreach ($tableselect['headers'] as $key => $value) {
      $tableselect['options']['none'][$key] = array(
        'data' => $key != 'title' ? NULL : l(t('None above, New content !'), 'node/add/' . str_replace('_', '-', str_replace(' ', '-', $scheme['content_type'])), array(
          'query' => array(
            'destination' => $destination,
            'edit[title]' => $form_state['storage']['title'],
          ),
        )),
      );
    }
  }
  $form['nid']['#header'] = $tableselect['headers'];
  $form['nid']['#options'] = $tableselect['options'];

  if (empty($tableselect['hidesubmit'])) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#weight' => 99,
    );
  }
  $form['pager'] = array(
    '#markup' => theme('pager', array('tags' => NULL)),
    '#weight' => 999,
  );
  unset ($tableselect);
  return $form;
}

/**
 * Implements the submit form of the add content form
 *
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 *
 * @return string
 *   message and redirect path of the submit action
 */
function dncsystem_add_contents_submit($form, &$form_state) {
  if (empty($form_state['values']['nid'])) {
    foreach ($form_state['values'] as $key => $value) {
      $form_state['storage'][$key] = $value;
    }
    $form_state['rebuild'] = TRUE;
    return;
  }
  form_state_values_clean($form_state);
  $scheme = $form_state['values']['scheme'];
  $options = array(
    'query' => array(
      'destination' => $form_state['values']['destination'],
    ),
  );
  if (empty($form_state['values']['nid']) || !is_numeric($form_state['values']['nid'])) {
    if ($form_state['values']['nid'] != 'none') {
      $dump = explode('|', $form_state['values']['nid']);
      if (count($dump) > 0) {
        foreach ($dump as $key => $value) {
          if (preg_match('/\:/', $value)) {
            list($ky, $vl) = explode(':', trim($value));
            $options['query'][trim($ky)] = trim($vl);
            unset ($ky, $vl);
          }
        }
      }
      unset ($dump);
    }
    else {
      $options['query']['edit[title]'] = empty($form_state['values']['title']) ? NULL : $form_state['values']['title'];
    }
    $form_state['values']['nid'] = 'none';
  }
  $form_state['redirect'] = array(
    $form_state['values']['nid'] != 'none' ? 'node/' . $form_state['values']['nid'] . '/edit' : 'node/add/' . str_replace('_', '-', str_replace(' ', '-', $scheme['content_type'])),
    $options,
  );
  unset ($scheme, $options);
}

/**
 * Generate data from custom module
 *
 * @param string $module
 *   name of the module
 * @param string $context
 *   string context e.g. 'persons', 'positions', etc. see
 *   $module . '_content_type_schema_vars
 * @param int $nid
 *   nid of the content
 * @param array|string $fields
 *   field limit to show
 * @param string $render
 *   render flag display formatter
 *
 * @return array
 *   data array if found or NULL if otherwise
 */
function dncsystem_content_get_data($module, $context, $nid = NULL, $fields = NULL, $render = NULL) {
  $function = $module . '_content_type_schema_vars';
  if (!function_exists($function)) {
    unset ($function);
    return;
  }
  $scheme = $function(TRUE, $context);
  if (empty($scheme['content_type'])) {
    unset ($scheme, $function);
    return;
  }
  unset ($function);
  $ids = empty($nid) ? FALSE : (!is_array($nid) ? array($nid) : $nid);
  $conditions = array(
    'type' => $scheme['content_type']
  );
  $result = entity_load('node', $ids, $conditions);
  unset ($ids, $conditions);
  if (empty($result)) {
    unset ($result, $scheme);
    return;
  }
  $return = array();
  foreach ($result as $node) {
    $return[$node->nid] = dncsystem_retrieve_node_data($scheme, $node, $fields, $render, TRUE);
  }
  unset ($result, $node, $scheme);
  return $return;
}

/**
 * Make updating node generally in the system
 *
 * @param string $module
 *   module name
 * @param string $context
 *   context of content type
 * @param array $data
 *   data array to update
 * @param boolean $force
 *   information to force action
 *
 * @return int
 *   node ID if success, or NULL if otherwise
 */
function dncsystem_content_update_data($module, $context, array $data = array(), $force = FALSE) {
  if (empty($data)) {
    return;
  }
  if (!empty($data['nid'])) {
    settype($data['nid'], 'int');
    if (!empty($data['nid'])) {
      $default = dncsystem_content_get_data($module, $context, $data['nid']);
      if (empty($default[$data['nid']])) {
        unset ($default);
        return;
      }
      $default = $default[$data['nid']];
      if (empty($force)) {
        // checking data
        foreach ($data as $keys => $values) {
          if (empty($default[$keys]) || $default[$keys] != $values) {
            $diff = TRUE;
            break;
          }
        }
        if (!isset($diff) || empty($diff)) {
          unset($diff, $default);
          return $data['nid'];
        }
        unset ($diff);
      }
    }
  }

  $function = $module . '_content_type_schema_vars';
  if (!function_exists($function)) {
    unset ($function, $default);
    return;
  }

  $scheme = $function(TRUE, $context);
  unset ($function);
  if (empty($scheme['content_type'])) {
    unset ($scheme, $default);
    return;
  }

  if (!empty($default)) {
    $node = node_load($default['nid']);
    if (empty($node->type) || $node->type != $scheme['content_type']) {
      unset ($node, $scheme, $default);
      return;
    }
    $node->status = NODE_PUBLISHED;
  }
  else {
    $node = new stdClass();
    $node->type = $scheme['content_type'];
    $node->title = empty($data['title']) ? t('Input in') . ' ' . dncsystem_date('l, Y-m-d H:i:s', time()) : $data['title'];
    $node->language = LANGUAGE_NONE;
    node_object_prepare($node);
    $node->uid = 1;
  }
  unset ($default);

//  watchdog('checkdata', '<pre>@printr</pre>', array('@printr' => print_r($data, TRUE)));
  foreach ($scheme['fields'] as $keys => $values) {
    if ($values == 'title') {
      if (isset($data[$keys])) {
        $node->title = $data[$keys];
      }
    }
    else {
      if (!isset($data[$keys]) || is_null($data[$keys])) {
        $node->{$values}[$node->language] = [];
        continue;
      }
      $data[$keys] = !is_array($data[$keys]) ? array($data[$keys]) : $data[$keys];
      $finfo = field_info_field($values);
      $column = isset($finfo['columns']['target_id']) ? 'target_id' : (isset($finfo['columns']['rgb']) ? 'rgb' : 'value');
      $i = 0;
      foreach ($data[$keys] as $value) {
        if ($finfo['type'] == 'file') {
          if (!empty($value) && !is_array($value)) {
            $value = (array)$value;
          }
          $value['display'] = empty($value['display']) ? TRUE : $value['display'];
          $node->{$values}[$node->language][$i] = $value;
        }
        else {
          $node->{$values}[$node->language][$i][$column] = $value;
          if ($keys == 'body') {
            $format = dncsystem_is_serialized($value) || dncsystem_is_validBase64($value) ? 'plain_text' : 'full_html';
            $node->{$values}[$node->language][$i]['format'] = $format;
            unset ($format);
          }
          elseif($finfo['type'] == 'entityreference') {
            $node->{$values}[$node->language][$i]['target_type'] = $finfo['settings']['target_type'];
          }
        }
        $i++;
      }
      unset ($i);
      unset ($finfo, $column);
//      watchdog('checkdatavalue', '<pre>@printr</pre>', array('@printr' => print_r($node->{$values}[$node->language], TRUE)));
    }
  }
  unset ($scheme);
  // watchdog('checkdnc', '<pre>@printr</pre>', array('@printr' => print_r($node, TRUE)));

  if (empty($node->nid)) {
    node_submit($node);
  }
  node_save($node);
  $return = $node->nid;
  unset ($node);
  return $return;
}

/**
 * Execute custom delete (fake)
 * just flag this by NODE NOT PUBLISHED ;)
 *
 * @param string $module
 *   the module name
 * @param string $context
 *   name of the context
 * @param int $nid
 *   nid of the content
 *
 * @return none
 */
function dncsystem_content_delete($module, $context, $nid) {
  $function = $module . '_content_type_schema_vars';
  if (!function_exists($function)) {
    unset ($function);
    return;
  }
  $scheme = $function(TRUE, $context);
  unset($function);
  if (empty($scheme['content_type'])) {
    unset($scheme);
    return;
  }
  $nids = !is_array($nid) ? array($nid) : $nid;
  foreach ($nids as $values) {
    $node = node_load($values);
    if (!empty($node->type) && $node->type == $scheme['content_type']) {
      $node->status = NODE_NOT_PUBLISHED;
      node_save($node);
    }
    unset ($node);
  }
}

function dncsystem_content_forced_get_variable($varname = NULL) {
  if (empty($varname)) {
    return;
  }

  $sql = db_select ('variable', 'v')
  ->fields('v')
  ->condition('name', $varname)
  ->execute();
  $result = $sql->fetchAll();
  unset ($sql);
  if (empty($result)) {
    unset ($result);
    return;
  }

  foreach ($result as $values) {
    return unserialize($values->value);
  }
  unset ($result);
  return;
}
