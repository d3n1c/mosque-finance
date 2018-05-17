<?php

/**
 * Converting integer in string format to big integer format
 * 
 * @param string $str
 *   integer in string formatted
 * 
 * @return int
 *   result in big integer formatted
 */
function dncsystem_string_convert_to_bigint($str) {
  $theint = preg_match('/^[0-9]+/', $str, $matches) ? $matches[0] : 0;
  if (PHP_INT_SIZE === 4) {
    settype($theint, 'float');
  }
  elseif (PHP_INT_SIZE === 8) {
    settype($theint, 'int');
  }
  return $theint;
}

function dncsystem_string_plain_clean($string, $reducews = TRUE) {
  $s = trim($string);
  $s = iconv("UTF-8", "UTF-8//IGNORE", $s); // drop all non utf-8 characters

  // this is some bad utf-8 byte sequence that makes mysql complain - control and formatting i think
  $s = preg_replace('/(?>[\x00-\x1F]|\xC2[\x80-\x9F]|\xE2[\x80-\x8F]{2}|\xE2\x80[\xA4-\xA8]|\xE2\x81[\x9F-\xAF])/', ' ', $s);

  $s = preg_replace('/\s+/', ' ', $s); // reduce all multiple whitespace to a single space

  return $s;
}

function dncsystem_string_convert_to_nospace_and_lower($string) {
  $string = preg_replace('/\s+/', '', $string);
  return strtolower($string);
}

function dncsystem_string_rgb_to_hex($string, $prefixsign = TRUE) {
  $data = explode(',', $string, 3);
  return (!empty($prefixsign) ? '#' : NULL) . sprintf("#%02x%02x%02x", $data[0], $data[1], $data[2]);
}

function dncsystem_string_hex_to_rgb($string) {
  list($r,$g,$b) = array_map('hexdec',str_split(ltrim($string, '#'),2));
  return $r . ',' . $g . ',' . $b;
}
