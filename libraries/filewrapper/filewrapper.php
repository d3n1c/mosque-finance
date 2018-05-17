<?php

Class FileWrap
{

    /**
     * Use to store fOpen connection
     * @type bool
     * @access private
     */
    private $handle;

    /**
     * To store the file URL/location
     * @type string
     * @access private
     */
    private $file;

    /**
     * To store the file URL/location
     * @type string
     * @access private
     */
    private $delfile;
    
    /**
     * To store the file URL/location
     * @type string
     * @access private
     */
    private $errormsg;
    
    /**
     * Used to initialize the file
     * @access public
     * @param string $file_url
     * File location/url
     * @example 'dir/mytext.txt'
     * @return bool
     */
    public function __construct($file_url) {
      if (empty($file_url)) {
        return;
      }
      $this->delfile = $file_url;
      $this->file = $file_url;
    }
    
    /**
     * Used to write inside the file,
     * If file doesn't exists it will create it
     * @access public
     * @param string $text
     * The text which we have to write
     * @example 'My text here in the file.';
     * @return bool
     */
    public function write($text) {
      if (!file_put_contents($this->file, $text, LOCK_EX)) {
        return FALSE;
      }
      return TRUE;
      if ($this->handle = fopen($this->file, 'c+')) {
        if (fwrite($this->handle, $text)) {
          fclose($this->handle);
          return true;
        } else {
          fclose($this->handle);
          $this->handle = NULL;
          return false;
        }
      }
    }

    /**
     * To read the contents of the file
     * @access public
     * @param bool $nl2br
     * By default set to false, if set to true will return
     * the contents of the file by preserving the data.
     * @example (true)
     * @return string|bool
     */
    public function read($nl2br = false) {
      if ($this->handle = fopen($this->file, 'c+')) {
        if ($read = fread($this->handle, filesize($this->file))) {
          if ($nl2br == true) {
            fclose($this->handle);
            $this->handle = NULL;
            return nl2br($read);
          }

          fclose($this->handle);
          $this->handle = NULL;
          return $read;
        } else {
          fclose($this->handle);
          $this->handle = NULL;
          return false;
        }
      }
    }

    /**
     * Use to delete the file
     * @access public
     * @return bool
     */
    public function delete() {
      $this->handle = NULL;

      if (file_exists($this->file)) {
        if (unlink($this->file)) {
          return true;
        } else {
          return false;
        }
      }
    }
    
    public function listing()
    {
      clearstatcache();
      if (!is_dir($this->file) && !if_file($this->file)) {
        return false;
      }
      
      if (is_file($this->file)) {
        return $this->read();
      }
      
      $result = [];
      $dump = scandir($this->file);
      if (!empty($dump)) {
        foreach ($dump as $values) {
          if ($values != '.' && $values != '..') {
            $result[] = $values;
          }
        }
      }
      unset ($dump);
      return $result;
    }
    
    public function makedir() {
      clearstatcache();
      if (is_dir($this->file) || is_file($this->file)) {
        return $this->file;
      }
      if (mkdir($this->file, 0775)) {
        return $this->file;
      }
      return FALSE;
    }

    public function deldir() {
      clearstatcache();
      if (is_dir($this->delfile) || is_file($this->delfile)) {
        if (!is_dir($this->delfile)) {
          unlink($this->delfile);
        }
        else {
          $dir = scandir($this->delfile);
          foreach ($dir as $values) {
            if ($values != '.' && $values != '..') {
              $path = $this->delfile . DIRECTORY_SEPARATOR . $values;
              clearstatcache();
              if (is_dir($path)) {
                $this->delfile = $path;
                $this->deldir();
              }
              else {
                unlink($path);
              }
            }
          }
          unset ($dir);
        }
      }
      if ($this->delfile == $this->file) {
        return TRUE;
      }
      $dump = explode(DIRECTORY_SEPARATOR, $this->delfile);
      unset ($dump[count($dump) - 1]);
      $this->delfile = implode(DIRECTORY_SEPARATOR, $dump);
      unset ($dump);
      $this->deldir();
    }
    
    // operations of files
    
    protected function checkSedCommand() {
      if (PHP_SHLIB_SUFFIX === 'dll') {
        return FALSE;
      }
      
      $command = shell_exec('command -v sed');
      if (empty($command)) {
        unset ($command);
        return FALSE;
      }
      unset ($command);
      return TRUE;
    }
    
    protected function prepareMainPath() {
      if (empty($this->file)) {
        return;
      }
      
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->makedir();
      }
    }
    
    public function maxid($type = NULL, bool $update = FALSE, array $data = []) {
      if (!$this->checkSedCommand() || empty($this->file)) {
        return;
      }
      $this->prepareMainPath();
      
      $maxids = [];
      
      $this->file .= DIRECTORY_SEPARATOR . 'maxids.json';
      clearstatcache();
      if (is_file($this->file)) {
        $maxids = $this->read();
        $maxids = json_decode($maxids, TRUE);
      }
      
      if (!empty($update)) {
        if (empty($data)) {
          return;
        }
        if (!empty($type) && empty($data[$type])) {
          return;
        }

        foreach ($data as $keys => $values) {
          $maxids[$keys] = $values;
        }
        
        $this->write(json_encode($maxids));
      }
      
      return empty($type) ? $maxids : (empty($maxids[$type]) ? 0 : $maxids[$type]);
    }
    
    public function getData($type, $nid) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($type) || empty($nid)) {
        return;
      }
      $this->prepareMainPath();
      
      $this->file .= DIRECTORY_SEPARATOR . $type;
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->file = NULL;
        return;
      }
      
      $this->file .= DIRECTORY_SEPARATOR . 'states';
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->file = NULL;
        return;
      }
      
      $this->file .= DIRECTORY_SEPARATOR . $nid . '.json';
      clearstatcache();
      if (!is_file($this->file)) {
        $this->file = NULL;
        return;
      }
      
      $result = $this->read();
      $this->file = NULL;
      $result = json_decode($result, TRUE);
      if (empty($result)) {
        unset ($result);
        return;
      }
      
      return $result;
    }

    public function dataListing($type, array $search = [], array $range = [], $justcount = FALSE, $reverse = FALSE) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($type)) {
        return;
      }
      $this->prepareMainPath();
      
      $this->file .= DIRECTORY_SEPARATOR . $type;
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->file = NULL;
        return;
      }
      
      $this->file .= DIRECTORY_SEPARATOR . 'index.txt';
      clearstatcache();
      if (!is_file($this->file)) {
        $this->file = NULL;
        return;
      }
      
      $max = shell_exec('sed -n \'$=\' ' . $this->file);
      $max = trim($max);
      settype($max, 'int');
      if (empty($max)) {
        unset ($max);
        return;
      }

      $range = empty($range) ? [0, 1000] : $range;
      settype($range[0], 'int');
      settype($range[1], 'int');
      $range[1] = empty($range[1]) ? 1 : $range[1];

      $result = [];
      if (!empty($search)) {
        if ($search['type'] == 'string') {
          if (!empty($justcount)) {
            unset ($max);
            $result = shell_exec('sed -n \'/' . $search['string'] . '/I=\' ' . $this->file . ' | wc -l');
            settype($result, 'int');
            return $result;
          }
          $lines = shell_exec('sed -n \'/' . $search['string'] . '/I=\' ' . $this->file);
        }
        else {
          if (empty($search['fields'])) {
            return $result;
          }
          $lines = [];
          $lcount = [];
          foreach ($search['fields'] as $keys => $values) {
            $dump = shell_exec('sed -n \'/"' . $keys . '":' . (is_numeric($values) ? $values : '"' . $values . '"') . '/I=\' ' . $this->file);
            if (!empty($dump)) {
              $dump = explode("\n", $dump);
              if (!empty($dump)) {
                foreach ($dump as $value) {
                  if (!empty($value)) {
                    $value = trim($value);
                    settype($value, 'int');
                    $lcount[$value] = empty($lcount[$value]) ? 1 : ($lcount[$value] + 1);
                  }
                }
              }
            }
            unset ($dump);
          }
          foreach ($lcount as $keys => $values) {
            if (empty($search['or'])) {
              if ($values >= count($search['fields'])) {
                $getit = TRUE;
              }
            }
            else {
              $getit = TRUE;
            }
            if (!empty($getit)) {
              if (!in_array($keys, $lines)) {
                $lines[] = $keys;
              }
            }
            unset ($getit);
          }
          unset ($lcount);

          if (!empty($justcount)) {
            return empty($lines) ? 0 : count($lines);
          }
        }
        if (!is_array($lines) && is_string($lines)) {
          $dump = explode("\n", $lines);
          $lines = [];
          foreach ($dump as $values) {
            $values = trim($values);
            settype($values, 'int');
            if (!in_array($values, $lines)) {
              $lines[] = $values;
            }
          }
          unset ($dump);
        }
        if (!empty($lines)) {
          if (empty($reverse)) {
            $istart = $range[0];
            $range[1] = $range[1] > count($lines) ? count($lines) : $range[1];
            $istop = $istart + $range[1];
          }
          else {
            $istart = count($lines) - ($range[0] + $range[1]);
            $istop = $istart + $range[1];
            if ($istart < 0) {
              $istart = 0;
            }
          }
          for ($i = $istart; $i < $istop; $i++) {
            if (!empty($lines[$i])) {
              $dump = shell_exec('sed \'' . $lines[$i] . '!d\' ' . $this->file);
              if (!empty($dump)) {
                $dump = json_decode($dump, TRUE);
                if (!empty($dump['nid'])) {
                  $dmp = $dump;
                  unset ($dmp['nid']);
                  if (!empty($dmp)) {
                    $result[] = $dump;
                  }
                  unset ($dmp);
                }
              }
              unset ($dump);
            }
          }
          unset ($i, $istart, $istop);
        }
        unset ($lines);

        ksort($result);
        if (!empty($reverse)) {
          krsort($result);
        }
        return $result;
      }

      if (!empty($justcount)) {
        return $max;
      }

      if (empty($reverse)) {
        $istart = $range[0];
        $istop = $istart + $range[1];
      }
      else {
        $istart = $max - ($range[0] + $range[1]);
        $istop = $istart + $range[1];
        if ($istart < 0) {
          $istart = 0;
        }
      }
      $istop = empty($istop) ? $max : ($istop > $max ? $max : $istop);
      unset ($max);

      for ($i = $istart; $i < $istop; $i++) {
        $buffer = shell_exec('sed \'' . ($i + 1) . '!d\' ' . $this->file);
        $buffer = trim($buffer);
        if (empty($buffer)) {
          unset ($buffer);
          continue;
        }

        $buffer = json_decode($buffer, TRUE);
        if (!empty($buffer['nid'])) {
          $dump = $buffer;
          unset($dump['nid']);
          if (!empty($dump)) {
            $result[] = $buffer;
          }
          unset ($dump);
        }
        unset ($buffer);
      }
      unset ($i, $istart, $istop);

      ksort($result);
      if (!empty($reverse)) {
        krsort($result);
        $return = [];
        foreach ($result as $values) {
          $return[] = $values;
        }
        unset ($result);
        return $return;
      }
      return $result;
    }
    
    protected function updateListing($type, $line = NULL, array $data = [], $remove = FALSE) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($type)) {
        return;
      }
      $this->prepareMainPath();
      
      if (empty($data) && empty($remove)) {
        return;
      }
      
      $this->file .= DIRECTORY_SEPARATOR . $type;
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->makedir();
      }
      
      $this->file .= DIRECTORY_SEPARATOR . 'index.txt';
      
      clearstatcache();
      if (!is_file($this->file)) {
        if (empty($remove) && !empty($data)) {
          $this->write(json_encode($data));
        }
        return;
      }

      if (empty($line)) {
        // adding value
        if (empty($remove) && !empty($data)) {
          file_put_contents($this->file, "\n" . json_encode($data), FILE_APPEND | LOCK_EX);
        }
        return;
      }

      if (!empty($remove)) {
        shell_exec('sed -i \'' . $line . 'd\' ' . $this->file);
        $result = TRUE;
      }
      else {
        if (!empty($data)) {
          shell_exec('sed -i \'' . $line . 's|^.*$|' . json_encode($data) . '|\' ' . $this->file);
          $result = TRUE;
        }
      }
      return empty($result) ? FALSE : TRUE;
    }
    
    protected function updateDataLog($type, array $data = []) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($type) || empty($data['nid'])) {
        return;
      }
      $this->prepareMainPath();
      
      $this->file .= DIRECTORY_SEPARATOR . $type;
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->makedir();
      }
      
      $this->file .= DIRECTORY_SEPARATOR . 'logs';
      clearstatcache();
      if (!is_dir($this->file)) {
        $this->makedir();
      }
      
      $this->file .= DIRECTORY_SEPARATOR . $data['nid'] . '.txt';
      clearstatcache();
      $text = (!is_file($this->file) ? NULL : "\n") . json_encode($data);
      file_put_contents($this->file, $text , FILE_APPEND | LOCK_EX);
      unset ($text);
      return TRUE;
    }
    
    protected function getListingLineByID($type, $nid) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($type)) {
        return;
      }
      $this->prepareMainPath();
      
      $this->file .= DIRECTORY_SEPARATOR . $type;
      clearstatcache();
      if (!is_dir($this->file)) {
        return;
      }
      
      $this->file .= DIRECTORY_SEPARATOR . 'index.txt';
      clearstatcache();
      if (!is_file($this->file)) {
        return;
      }
      
      $result = shell_exec('sed -n \'/,"nid":' . $nid . '[,\}]/=\' ' . $this->file);
      if (empty($result)) {
        unset ($result);
        return;
      }
      $result = trim($result);
      settype($result, 'int');
      return $result;
    }

    public function updateData($uid, array $data = []) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($uid)) {
        return;
      }
      $this->prepareMainPath();
      
      $result = [];
      $maxid = $this->maxid();
      $this->file = $this->delfile;
      foreach ($data as $keys => $values) {
        if (empty($values)) {
          continue;
        }
        
        $this->file .= DIRECTORY_SEPARATOR . $keys;
        
        clearstatcache();
        if (!is_dir($this->file)) {
          $this->makedir();
        }
        
        $typepath = $this->file;
        foreach ($values as $key => $value) {
          $this->file .= DIRECTORY_SEPARATOR . 'states';
          clearstatcache();
          if (!is_dir($this->file)) {
            $this->makedir();
          }
          
          if (empty($value['nid'])) {
            $value['nid'] = empty($maxid[$keys]) ? 1 : ($maxid[$keys] + 1);
            $maxid[$keys] = $value['nid'];
            $this->file .= DIRECTORY_SEPARATOR . $value['nid'] . '.json';
            $value['creator'] = $uid;
            $value['created'] = time();
          }
          else {
            $this->file .= DIRECTORY_SEPARATOR . $value['nid'] . '.json';
            $default = $this->read();
            $default = json_decode($default, TRUE);
            $default = empty($default) ? [] : $default;
            foreach ($value as $ke => $val) {
              $default[$ke] = $val;
            }
            $value = $default;
            unset ($default);
          }
          
          $value['updater'] = $uid;
          $value['updated'] = time();
          foreach (['nid', 'creator', 'created', 'updater', 'updated'] as $val) {
            settype($value[$val], 'int');
          }
          $this->write(json_encode($value));
          
          // update log
          $this->file = $this->delfile;
          $this->updateDataLog($keys, $value);
          
          // update listing
          $this->file = $this->delfile;
          $line = $this->getListingLineByID($keys, $value['nid']);
          $this->file = $this->delfile;
          $this->updateListing($keys, $line, $value);
          unset ($line);
          
          $result[$keys][$key] = $value;
          
          $this->file = $typepath;
        }
        unset ($typepath);
        
        $this->file = $this->delfile;
      }
      $this->maxid(NULL, TRUE, $maxid);
      unset ($maxid);
      
      return $result;
    }
    
    public function deleteData($uid, array $data = []) {
      if (!$this->checkSedCommand() || empty($this->file) || empty($uid) || empty($data)) {
        return;
      }
      $this->prepareMainPath();
      
      foreach ($data as $keys => $values) {
        if (empty($values)) {
          continue;
        }
        
        $this->file .= DIRECTORY_SEPARATOR . $keys;
        clearstatcache();
        if (!is_dir($this->file)) {
          $this->file = $this->delfile;
          continue;
        }
        
        $typefile = $this->file;
        foreach ($values as $key => $value) {
          if (empty($value['nid'])) {
            continue;
          }
          
          $this->file .= DIRECTORY_SEPARATOR . 'states';
          clearstatcache();
          if (!is_dir($this->file)) {
            $this->file = $typefile;
            continue;
          }
          $statefile = $this->file;
          
          $this->file .= DIRECTORY_SEPARATOR . $value['nid'] . '.json';
          clearstatcache();
          if (!is_file($this->file)) {
            $this->file = $typefile;
            continue;
          }
          unlink($this->file);
          
          // check states count and remove if empty
          $this->file = $statefile;
          $list = $this->listing();
          if (count($list) < 1) {
            rmdir($this->file);
          }
          unset ($statefile, $list);
          
          // log the action
          $value['deleted'] = time();
          $value['deleter'] = $uid;
          foreach (['nid', 'deleter', 'deleted'] as $val) {
            settype($value[$val], 'int');
          }
          
          $this->file = $this->delfile;
          $this->updateDataLog($keys, $value);
          
          // update data listing
          $this->file = $this->delfile;
          $line = $this->getListingLineByID($keys, $value['nid']);
          $this->file = $this->delfile;
          $this->updateListing($keys, $line, $value, TRUE);
          unset ($line);
          
          $this->file = $typefile;
        }
        unset ($typefile);
        
        $this->file = $this->delfile;
      }
      
      return TRUE;
    }

    /*
   * 
   * Examples
   * 

    // Sample text
    $text = <<<file
    Name: Ashwin Pathak
    Age: 15 years
    Country: India
    Blog Url: http://codicious.blogspot.com
    file;

    // Creating an Instance
    $file = new FileWrap('text.txt');

    // writing
    if ($file->write($text))
    {
        echo '<b>Status: </b>Wrote successfully!<br /><br />';
    }

    // reading
    if ($read = $file->read(true))
    {
        echo '<b>Status: Reading</b><br />' . $read;
    }

    // deleting
    if ($file->delete())
    {
        echo '<br /><br /><b>Status: </b>Deleted successfully!';
    }

        Post URL:   http://codicious.blogspot.com
   * 
   */
}
