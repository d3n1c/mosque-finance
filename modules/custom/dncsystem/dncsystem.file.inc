<?php

function dncsystem_file_get_max_lines ($file) {
  $realpath = $file;

  if (is_object($file) && !empty($file->uri)) {
    $realpath = drupal_realpath($file->uri);
  }
  $tfile = new SplFileObject($realpath);
  $tfile->seek(PHP_INT_MAX);
  $lines = $tfile->key();
  unset ($tfile);

  return $lines;
}

function dncsystem_file_line_buffer($file, $line, $reducews = TRUE) {
  $max = dncsystem_file_get_max_lines($file);
  if ($line < 0 || $line > $max) {
    unset ($max);
    return;
  }
  unset ($max);
  $file = is_object($file) && !empty($file->uri) ? drupal_realpath($file->uri) : $file;
  $splfile = new SplFileObject($file);
  $splfile->seek($line);
  $buffer = trim($splfile->current());
  unset ($splfile);

  module_load_include('inc', 'dncsystem', 'dncsystem.string');
  return empty($buffer) ? NULL : (empty($reducews) ? trim($buffer) : dncsystem_string_plain_clean($string));
}

function dncsystem_file_get_object_by_uri($uri) {
  $uri = file_stream_wrapper_uri_normalize($uri);
  // We should always attempt to re-use a file if possible.
  $files = entity_load('file', FALSE, array('uri' => $uri));
  $file = !empty($files) ? reset($files) : FALSE;
  unset ($files);

  if (empty($file)) {
    $file = new stdClass();
    $file->uid = $GLOBALS['user']->uid;
    $file->filename = basename($uri);
    $file->uri = $uri;
    $file->filemime = file_get_mimetype($uri);
    // This is gagged because some uris will not support it.
    $file->filesize = @filesize($uri);
    $file->timestamp = REQUEST_TIME;
    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
  }

  return $file;
}

function dncsystem_file_save_files($files, $schemepath) {
  $saved_files = array();
  $scheme = $schemepath;
  foreach ($files as $uploaded_file) {
    if ($uploaded_file['status'] == 'done') {
      $source = $uploaded_file['tmppath'];
      $destination = file_stream_wrapper_uri_normalize($scheme . '/' . $uploaded_file['name']);
      $destination = file_unmanaged_move($source, $destination, FILE_EXISTS_REPLACE);
      $file = dncsystem_file_get_object_by_uri($destination);
      // Check the $file object to see if all necessary properties are set. Otherwise, use file_load($file->fid) to populate all of them.
      $saved_files[] = $file;
    }
  }

  return $saved_files;
}

/**
 *
 * param string $url
 *    Full url to file to download
 * param string $uri
 *    Drupal uri of where to save file public://archive/test.pdf
 * param int $save_mode
 *    File save mode from drupal core, ex FILE_EXISTS_REPLACE
 *
 * Thanks to Patrick J Waters
 * http://drupalsun.com/patrick/2013/08/22/drupal-7-how-programmatically-download-files-optionally-save-them-managed-files
 */
function dncsystem_download_external_file($url, $uri, $manage_file = TRUE, $save_mode = FILE_EXISTS_RENAME) {

  $url_info = parse_url($url);
  $url_path_info = pathinfo($url_info['path']);

  //This helps with filenames with spaces
  $url = $url_info['scheme'] . '://' . $url_info['host']  . $url_path_info['dirname'] .'/'. rawurlencode($url_path_info['basename']);
  watchdog('getfile', '@printr', array('@printr' => $url));

  //Need to remove the filename from the uri
  $uri_target = file_uri_target($uri);
  $uri_scheme = file_uri_scheme($uri);
  $uri_path_info = pathinfo($uri_target);
  $directory = file_stream_wrapper_uri_normalize($uri_scheme . "://" . $uri_path_info['dirname']);

  if(file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    $drupal_result = drupal_http_request($url);
    if(!empty($drupal_result->data)) {
      $path = file_stream_wrapper_uri_normalize($uri);
      if($manage_file) {
        $new_file = file_save_data($drupal_result->data, $path, $save_mode);
      } else {
        return file_unmanaged_save_data($drupal_result->data, $path, $save_mode);
      }
    } else {
      drupal_set_message("Error downloading file, no data recieved for " . $url);
      return FALSE;
    }

    $new_file->display = 1;
    return (array)$new_file;
  } else {
    drupal_set_message("Could not create directory");
    return FALSE;
  }
}

function dncsystem_file_combine_chunks($realpath, array $chunks = array(), $absolutename) {
  if (empty($chunks)) {
    return;
  }

  clearstatcache();
  if (!is_dir($realpath)) {
    return;
  }

  $command = 'cd ' . $realpath . '; /usr/bin/cat * > ../' . escapeshellarg(basename($absolutename));
  shell_exec($command);
  unset ($command);
  // $handle = fopen($absolutename, 'w');
  // foreach ($chunks as $values) {
  //   $chunkpath = $realpath . DIRECTORY_SEPARATOR . $values;
  //   clearstatcache();
  //   if (!is_file($chunkpath)) {
  //     fclose($handle);
  //     unset ($chunkpath, $handle);
  //     return;
  //   }
  //   fwrite($handle, file_get_contents($chunkpath));
  //   unset ($chunkpath);
  // }
  // fclose($handle);
  // unset ($handle);
  return TRUE;
}
