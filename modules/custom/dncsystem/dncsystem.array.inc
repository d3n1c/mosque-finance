<?php

function dncsystem_array_get_data_chunck(array $data = array(), $limit) {
  if (empty($data) || empty($limit)) {
    return $data;
  }

  if (count($data) < $limit) {
    return $data;
  }

  $total = count($data);
  $current_page = pager_default_initialize($total, $limit);
  unset ($total);
  $chunks = array_chunk($data, $limit, TRUE);
  return $chunks[$current_page];
}
