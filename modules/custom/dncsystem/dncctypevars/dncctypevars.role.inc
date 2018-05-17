<?php

function dncctypevars_bulk_assign_node_permission(array $form = array(), array $form_state = array()) {
  $form['title'] = array(
    '#type' => 'item',
    '#weight' => 5,
    '#markup' => '<strong>' . t('One way bulk assigning node permissions by choosed role') . '</strong>',
  );
  $form['role'] = array(
    '#type' => 'select',
    '#title' => t('Role'),
    '#description' => t('Select role that will assign to node permission'),
    '#options' => user_roles(),
    '#weight' => 10,
    '#size' => 1,
    '#required' => TRUE,
  );
  $form['permissions'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Permission Types'),
    '#description' => t('Choose type of permissions of targeted node'),
    '#options' => array(
      'create' => t('Create'),
      'edit own' => t('Edit Own'),
      'edit any' => t('Edit Any'),
      'delete own' => t('Delete Own'),
      'delete any' => t('Delete Any'),
    ),
    '#weight' => 20,
  );
  
  $form['nodetypes'] = array(
    '#type' => 'tableselect',
    '#title' => t('Node Types'),
    '#caption' => t('Node Types'),
    '#empty' => t('No content type found :('),
    '#weight' => 30,
    '#options' => array(),
    '#header' => array(
      'title' => array('data' => t('Title'), 'width' => '25%'),
      'description' => array('data' => t('Description')),
    ),
  );
  $types = node_type_get_types();
  if (!empty($types)) {
    foreach ($types as $keys => $values) {
      $form['nodetypes']['#options'][$keys]['title']['data'] = $values->name;
      $form['nodetypes']['#options'][$keys]['description']['data'] = $values->description;
    }
  }
  unset ($types);
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('assign'),
    '#weight' => 200,
  );
  return $form;
}

function dncctypevars_bulk_assign_node_permission_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $form_state['redirect'] = 'admin/people/permissions';
  drupal_set_message(t('Node type roles have been successfully configured :)'));
  if (empty($form_state['input']['nodetypes'])) {
    return;
  }
  if (empty($form_state['input']['permissions'])) {
    return;
  }
  $role = user_role_load($form_state['input']['role']);
  if (empty($role->rid)) {
    unset ($role);
    return;
  }
  $revokes = array();
  $permissions = array();
  foreach ($form_state['input']['nodetypes'] as $keys => $values) {
    foreach ($form_state['input']['permissions'] as $key => $value) {
      $revokes[] = $key . ' ' . $keys . ' content';
    }
    if (!empty($values)) {
      foreach ($form_state['input']['permissions'] as $key => $value) {
        if (!empty($value)) {
          $permissions[] = $key . ' ' . $keys . ' content';
        }
      }
    }
  }
  user_role_revoke_permissions($role->rid, $revokes);
  user_role_grant_permissions($role->rid, $permissions);
  unset ($role, $permissions);
}
