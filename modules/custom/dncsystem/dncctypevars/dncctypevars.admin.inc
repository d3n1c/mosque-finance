<?php

/* 
 * Copyright (C) 2015 denic
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * Some of administration functions
 */

/**
 * Listing of registered variables to manage later
 * Implements hook_form
 * 
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   varaible array of the form_state
 * 
 * @return array
 *   array of manage form
 */
function dncctypevars_manager(array $form = array(), array $form_state = array()) {
  if (!empty($form_state['storage']['confirmed'])) {
    foreach ($form_state['storage'] as $key => $value) {
      $form[$key] = array(
        '#type' => 'value',
        '#value' => $value,
      );
    }
    $description = '<ul>';
    foreach ($form_state['storage']['items'] as $value) {
      $infos = dncctypevars_get_variables_by_id($value);
      if (!empty($infos)) {
        $description .= '<li>' . $infos['parentkey'] . '/' . $infos['varname'] . ': ' . $infos['varvalue'] . ' (context: ' . $infos['context'] . ', owned by: ' . $infos['module'] . ')</li>';
      }
      unset ($infos);
    }
    $description .= '</ul>';
    return confirm_form(
            $form,
            t('Are you sure want to delete these items ?'),
            current_path(),
            $description,
            t('Yes delete all these items now!'),
            t('CANCEL!')
          );
  }
  $header = array(
    'varid' =>
      array(
        'data' => '',
        'field' => 'varid',
        'style' => 'display: none;',
        'sort' => 'DESC',
      ),
    'module' =>
      array(
        'data' => t('Module'),
        'field' => 'module'
      ),
    'parentkey' =>
      array(
        'data' => t('Parent Key'),
        'field' => 'parentkey',
      ),
    'context' =>
      array(
        'data' => t('Context'),
        'field' => 'context',
      ),
    'varname' =>
      array(
        'data' => t('Name'),
        'field' => 'varname',
      ),
    'varvalue' =>
      array(
        'data' => t('Value'),
        'field' => 'varvalue',
      ),
  );
  $query = db_select('dncctypevars_registers', 'n')->extend('PagerDefault')
          ->fields('n');
//  $query->orderBy('varid', 'DESC');
  if (!empty($form_state['storage']['search'])) {
    $string = $form_state['storage']['search'];
    $db_or = db_or();
    $db_or->condition('varname', '%' . $string . '%', 'LIKE');
    $db_or->condition('module', '%' . $string . '%', 'LIKE');
    $db_or->condition('parentkey', '%' . $string . '%', 'LIKE');
    $query->condition($db_or);
    unset ($db_or, $string);
  }
  else {
    $query->limit(10);
    $query->extend('TableSort')->orderByHeader($header);
  }
  $query = $query->execute();
  $result = $query->fetchAll();
  
  $options = array();
  if (count($result) > 0) {
    foreach ($result as $value) {
      $options[$value->varid] = array(
        'varid' => array(
          'data' => $value->varid,
          'style' => 'display: none;',
        ),
        'module' => array(
          'data' => $value->module,
        ),
        'parentkey' => array(
          'data' => $value->parentkey,
        ),
        'context' => array(
          'data' => $value->context,
        ),
        'varname' => array(
          'data' => $value->varname,
        ),
        'varvalue' => array(
          'data' => $value->varvalue,
        ),
      );
    }
  }
  
  $form['items'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No variables be registered'),
  );
  if (!empty($options) && count($options) > 0) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete selected items')
    );
  }
  $form['pager'] = array(
    '#markup' => theme('pager'),
  );
  
  return $form;
}

/**
 * Implements hook_submit
 * 
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 * 
 * @return string
 *   redirection path if done
 */
function dncctypevars_manager_submit(&$form, &$form_state) {
  if (empty($form_state['values']['confirmed'])) {
    $form_state['storage']['confirmed'] = TRUE;
    $form_state['storage']['items'] = array();
    foreach ($form_state['values']['items'] as $value) {
      if (!empty($value)) {
        $form_state['storage']['items'][] = $value;
      }
    }
    if (count($form_state['storage']['items']) < 1) {
      unset ($form_state['storage']['items'], $form_state['storage']['confirmed']);
      $form_state['redirect'] = current_path();
      return;
    }
    $form_state['rebuild'] = TRUE;
    return;
  }
  
  dncctypevars_delete_variables_by_ids($form_state['values']['items']);
  drupal_set_message(t('The selected variables have been deleted successfully :)'));
  $form_state['redirect'] = current_path();
}

/**
 * Registering variable form
 * 
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 * 
 * @return array
 *   form array
 */
function dncctypevars_register(array $form = array(), array $form_state = array()) {
  $form['module'] = array(
    '#type' => 'textfield',
    '#title' => t('Module'),
    '#description' => t('Fill by module name that will use this variable'),
    '#size' => 40,
    '#required' => TRUE,
    '#autocomplete_path' => 'dncctypevars/search/module'
  );
  $form['parentkey'] = array(
    '#type' => 'textfield',
    '#title' => t('Parent Key'),
    '#description' => t('Name of parent term of content type that will be used by module environment'),
    '#size' => 40,
    '#required' => TRUE,
    '#autocomplete_path' => 'dncctypevars/search/parentkey'
  );
  $form['context'] = array(
    '#type' => 'radios',
    '#title' => t('Context'),
    '#description' => t('context of variable whether content type or field'),
    '#required' => TRUE,
    '#options' => array('content_type' => t('Content Type'), 'field' => t('Field')),
    '#default_value' => 'content_type',
  );
  $form['varname'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Name of variable'),
    '#size' => 40,
    '#required' => TRUE,
  );
  $form['ctvalue'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#description' => t('Value of variable'),
    '#size' => 40,
    '#autocomplete_path' => 'dncctypevars/search/contenttype',
    '#states' => array(
      'visible' => array(
        ':input[name="context"]' => array('value' => 'content_type'),
      ),
    ),
  );
  $form['flvalue'] = array(
    '#type' => 'textfield',
    '#title' => t('Value'),
    '#description' => t('Value of variable'),
    '#size' => 40,
    '#autocomplete_path' => 'dncctypevars/search/field',
    '#states' => array(
      'visible' => array(
        ':input[name="context"]' => array('value' => 'field'),
      ),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Register')
  );
  
  return $form;
}

/**
 * Implements of hook_validate
 * 
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 */
function dncctypevars_register_validate(&$form, &$form_state) {
  if ($form_state['values']['context'] == 'content_type') {
    if (empty($form_state['values']['ctvalue'])) {
      form_set_error('ctvalue', t('Value of variable is required!'));
    }
  }
  else {
    if (empty($form_state['values']['flvalue'])) {
      form_set_error('flvalue', t('Value of variable is required!'));
    }
  }
}

/**
 * Implements hook_submit for register form
 * 
 * @param array $form
 *   variable array of the form
 * @param array $form_state
 *   variable array of the form_state
 */
function dncctypevars_register_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $record = new stdClass();
  $fields = drupal_schema_fields_sql('dncctypevars_registers');
  foreach ($form_state['values'] as $key => $value) {
    if (in_array($key, $fields)) {
      $record->$key = $value;
    }
  }
  $record->varid = dncctypevars_get_new_variable_id();
  $record->varvalue = $record->context == 'content_type' ? $form_state['values']['ctvalue'] : $form_state['values']['flvalue'];
  drupal_write_record('dncctypevars_registers', $record);
  unset ($record);
  drupal_set_message(t('New variable has been registered'));
  $form_state['redirect'] = drupal_get_path_alias('admin/structure/dncsystem/dncctypevars');
}

/**
 * Manage to converting data structure to content type and all the fieldnames
 * Implements hook_form
 * 
 * @param array $form
 *   variable array of form
 * @param array $form_state
 *   variable array of form_state
 * @param type $module
 *   module that having the schema structures
 * 
 * @return array
 *   form array that will rendered by drupal_get_form function
 */
function dncctypevars_module_register(array $form = array(), array $form_state = array(), $module) {
  $form['module'] = array(
    '#type' => 'value',
    '#value' => $module,
  );
  $modulename = system_get_info('module', $module);
  $form['moduletitle'] = array(
    '#type' => 'item',
    '#markup' => '<strong>' . ucwords(strtolower(t('Content Type Variables of'))) . ' ' . (empty($modulename['name']) ? t('General') : $modulename['name']) . '</strong>',
    '#weight' => -90,
  );
  unset ($modulename);
  ctools_include('ajax');
  ctools_include('modal');
  ctools_modal_add_js();
  dncctypevars_set_modal_settings('medium');
  $function = $module . '_content_type_schema_vars';
  if (!function_exists($function)) {
    unset ($function);
    drupal_set_message(t('Unknown scripting error, please contact @them module developers', array('@them' => $module)), 'error', FALSE);
    return $form;
  }
  $sources = $function();
  if (empty($sources)) {
    unset ($sources, $function);
    drupal_set_message(t('Source variables not found in the function :('), 'error', FALSE);
    return $form;
  }
  $ctypes = array();
  $types = node_type_get_types();
  if (!empty($types)) {
    foreach ($types as $value) {
      $ctypes[$value->type] = $value->name;
    } 
  }
  unset ($types);
  $contexts = array();
  $i = 0;
  foreach ($sources as $keys => $values) {
    $defaults = $function(TRUE, $keys);
    if (!empty($defaults['content_type'])) {
      $form_state['storage']['parentkey_' . $keys] = $defaults['content_type'];
    }
    $form['manage_' . $keys] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => '<strong>' . t('Manage') . '</strong> ' . $keys . (empty($defaults['content_type']) ? NULL : ' => ' . $defaults['content_type']),
      '#weight' => $i * 10,
    );
    $editlink = ctools_modal_text_button(
            '<button type="button">' . t('choose another content type') . '</button>',
            'dncctypevars/ctset/nojs/form/setctype/' . $module . '/' . $keys,
            t('Choose Content Type'),
            'ctools-modal-dncctypevars-modal-style'
    );
  
    if (!empty($form_state['storage']['parentkey_' . $keys])) {
      $form['ctype_' . $keys] = array(
        '#type' => 'value',
        '#value' => $form_state['storage']['parentkey_' . $keys],
      );
    }
    $form['manage_' . $keys]['content_type_of_' . $keys] = array(
      '#type' => 'item',
      '#title' => t('Content Type') . ': ',
      '#description' => ' (' . t('The <strong>@dkeys</strong> data will be mapped to content type', array('@dkeys' => $keys)) . ')',
      '#markup' => empty($defaults['content_type']) ? key($ctypes) : $ctypes[$defaults['content_type']],
      '#weight' => 0,
      '#prefix' => '<div class="container-inline">',
    );
    $form['manage_' . $keys]['update_ctype_of_' . $keys] = array(
      '#type' => 'item',
      '#markup' => $editlink,
      '#weight' => 1,
      '#suffix' => '</div>',
    );
    unset ($editlink);
    ${'contexts_' . $keys} = empty($defaults['content_type']) ? key($ctypes) : $defaults['content_type'];
    $form['manage_' . $keys]['fields_of_' . $keys] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#title' => t('Fields of') . ' ' . $keys,
      '#weight' => 2,
    );
    $options = array();
    $getnode = node_type_get_type($defaults['content_type']);
    $options['title'] = empty($getnode->title_label) ? t('Title') : $getnode->title_label;
    unset ($getnode);
    ${'fields_of_' . $keys} = field_info_instances('node', ${'contexts_' . $keys});
    if (!empty(${'fields_of_' . $keys})) {
      foreach (${'fields_of_' . $keys} as $value) {
        $options[$value['field_name']] = $value['label'];
      }
    }
    $u = 0;
    foreach ($values as $value) {
      $form['manage_' . $keys]['fields_of_' . $keys]['field_' . $keys . '_' . $value] = array(
        '#type' => 'select',
        '#options' => $options,
        '#title' => $value . ': ',
        '#weight' => $u,
        '#default_value' => empty($defaults['fields'][$value]) ? key($options) : $defaults['fields'][$value],
      );
      $u++;
    }
    unset ($u, $options, $defaults);
    $i++;
  }
  unset ($i, $ctypes);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 500,
  );
  return $form;
}

/**
 * Submit action for module register form
 * 
 * @param array $form
 *   variable array of form
 * @param array $form_state
 *   variable array of form state
 */
function dncctypevars_module_register_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
//  echo '<pre>'; print_r($form_state['values']); echo '</pre>'; exit;
  $function = $form_state['values']['module'] . '_content_type_schema_vars';
  $source = $function();
  foreach ($source as $keys => $values) {
    $sourcekey = 'ctype_' . $keys;
    if (!empty($form_state['values'][$sourcekey])) {
      dncctypevars_save_variable(
        $form_state['values']['module'],
        $keys,
        $keys,
        'content_type',
        $form_state['values'][$sourcekey]
      );
    }
    unset ($sourcekey);
    foreach ($values as $value) {
      $sourcekey = 'field_' . $keys . '_' . $value;
      if (!empty($form_state['values'][$sourcekey])) {
        dncctypevars_save_variable(
          $form_state['values']['module'],
          $value,
          $keys,
          'field',
          $form_state['values'][$sourcekey]
        );
      }
      unset ($sourcekey);
    }
  }
  unset ($function, $source);
  drupal_set_message(t('Variables saved successfully'));
  $form_state['redirect'] = current_path();
}

/**
 * The callback of modal popup form to change content type variable
 * 
 * @param boolean $js
 *   info whether use js or no, default to NULL
 * @param string $module
 *   module name
 * @param string $context
 *   context of the variable, always content_type
 * 
 * @return array
 *   ajax rendering
 */
function dncctypevars_module_register_ctype_popup($js = NULL, $module, $context) {
  // degrade half-assed to no js
  $contexts = array(
    'module' => $module,
    'context' => $context,
  );
  if(!$js) {
    $contexts = urlencode(base64_encode(serialize($contexts)));
    return drupal_get_form('dncctypevars_register_content_type', $contexts);
  }
  
  // adelante!
  ctools_include('modal');
  ctools_include('ajax');
  $form_state['title'] = t('Register Content Type for') . ' ' . ucwords(str_replace('_', ' ', $context));
  $form_state['ajax'] = TRUE;
//  $form_state['storage']['usectools'] = TRUE;
  ctools_include('object-cache');
  $object = ctools_object_cache_get('register_submission', 'storages');
  $form_state['storage']['context'] = $contexts;
  $output = ctools_modal_form_wrapper('dncctypevars_register_content_type', $form_state);
  if (!empty($form_state['executed'])) {
    // We'll just overwrite the form output if it was successful.
    ctools_include('object-cache');
    $object = ctools_object_cache_get('register_submission', 'storages');
    foreach ($form_state['values'] as $key => $value) {
      $object->{$key} = $value;
    }
    ctools_object_cache_set('register_submission', 'storages', $object);
    $output = array();
    $output[] = ctools_modal_command_dismiss(t('Success!'));
    $output[] = ctools_ajax_command_reload();
  }
  print ajax_render($output);
}
