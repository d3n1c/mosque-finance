<?php

/*

Copyright (c) 2010, dealnews.com, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of dealnews.com, Inc. nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

 */

/**
 * show a status bar in the console
 *
 * <code>
 * for($x=1;$x<=100;$x++){
 *
 *     show_status($x, 100);
 *
 *     usleep(100000);
 *
 * }
 * </code>
 *
 * @param   int     $done   how many items are completed
 * @param   int     $total  how many items are to be done total
 * @param   int     $size   optional size of the status bar
 * @return  void
 *
 */

function dncsystem_barcli_show_status($done, $total, $size=30, $lineWidth=-1) {
    if($lineWidth <= 0){
        $lineWidth = $_ENV['COLUMNS'];
    }

    static $start_time;

    // to take account for [ and ]
    $size -= 3;
    // if we go over our bound, just ignore it
    if($done > $total) return;

    if(empty($start_time)) $start_time=time();
    $now = time();

    $perc=(double)($done/$total);

    $bar=floor($perc*$size);

    // jump to the begining
    echo "\r";
    // jump a line up
    echo "\x1b[A";

    $status_bar="[";
    $status_bar.=str_repeat("=", $bar);
    if($bar<$size){
        $status_bar.=">";
        $status_bar.=str_repeat(" ", $size-$bar);
    } else {
        $status_bar.="=";
    }

    $disp=number_format($perc*100, 0);

    $status_bar.="]";
    $details = "$disp%  $done/$total";

    $rate = ($now-$start_time)/$done;
    $left = $total - $done;
    $eta = round($rate * $left, 2);

    $elapsed = $now - $start_time;


    $details .= " " . dncsystem_barcli_setformatTime($eta) . " " . dncsystem_barcli_setformatTime($elapsed);

    $lineWidth--;
    if(strlen($details) >= $lineWidth){
        $details = substr($details, 0, $lineWidth-1);
    }
    echo "$details\n$status_bar";

    flush();

    // when done, send a newline
    if($done == $total) {
        echo "\n";
    }
}

function dncsystem_barcli_setformatTime($sec){
    if($sec > 100){
        $sec /= 60;
        if($sec > 100){
            $sec /= 60;
            return number_format($sec) . " hr";
        }
        return number_format($sec) . " min";
    }
    return number_format($sec) . " sec";
}


class dncsystemBarcliTimer {
    public $time;
    function __construct(){
        $this->start();
    }
    function start($offset=0){
        $this->time = microtime(true) + $offset;
    }
    function seconds(){
        return microtime(true) - $this->time;
    }
};


// We need this to limit the frequency of the progress bar. Or else it
// hugely slows down the app.
class dncsystemBarcliFPSLimit {
    public $frequency;
    public $maxDt;
    public $timer;
    function __construct($freq){
        $this->setFrequency($freq);
        $this->timer = new dncsystemBarcliTimer();
        $this->timer->start();
    }
    function setFrequency($freq){
        $this->frequency = $freq;
        $this->maxDt = 1.0/$freq;
    }
    function frame(){
        $dt = $this->timer->seconds();
        if($dt > $this->maxDt){
            $this->timer->start($dt - $this->maxDt);
            return true;
        }
        return false;
    }
};

class dncsystemBarcliProgress {
    // generic progress class to update different things
    function update($units, $total){}
}

class dncsystemBarcliSimpleProgress extends dncsystemBarcliProgress {
    private $cols;
    private $limiter;
    private $units;
    private $total;

    function __construct(){
        // change the fps limit as needed
        $this->limiter = new dncsystemBarcliFPSLimit(10);
        echo "\n";
    }

    function __destruct(){
        $this->draw();
    }

    function updateSize(){
        // get the number of columns
        $this->cols = exec("tput cols");
    }

    function draw(){
        $this->updateSize();
        dncsystem_barcli_show_status($this->units, $this->total, $this->cols, $this->cols);
    }

    function update($units, $total){
        $this->units = $units;
        $this->total = $total;
        if(!$this->limiter->frame())
            return;
        $this->draw();
    }
}
