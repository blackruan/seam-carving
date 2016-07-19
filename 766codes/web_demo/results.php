<!DOCTYPE html>
<html>

<head>
  <title> seam carved results </title>
  <style type='text/css'>
    a:link { color: #000fff; }
    a:visited { color: #000fff; }
    a:hover { color: #0099ff; }
    a:active { color: #000fff; }
  </style> 
</head>

<body bgcolor="#FFFFF5">
<font face="Comic sans MS">

<?php

if (isset($_POST["select_filetype"]) && isset($_POST["select_filename"])) {
  
  // define location
  $locs= "uploads/";
  $res= "results/";
  
  // get partial info
  $ftype= $_POST["select_filetype"];
  $fname= $_POST["select_filename"];
  $width= $_POST["select_width"];
  $height= $_POST["select_height"];
  
  // detect if it is image or video
  if ($ftype === 'image') { 
    
    // define parameters
    $method= $_POST["select_method"];
    $energy= $_POST["select_energy"];
    $ofilename= "carved-".$fname;
    
    // define program name and command
    $program= "./seam-carving ";
    $command= $program.$fname." ".$height." ".$width." ".$energy." n";
  
    // execute command   
    $e= exec($command);
    
    echo '
  <h2> Seam Carved Results </h2>
  
  <table>
    <tr>
      <td> <h3> Original </h3>
    </tr>
    <tr>
      <td> <img src="'.$fname.'"/>
    </tr>
    <tr>
      <td> <h3> Carved </h3>
    </tr>
    <tr>
      <td> <img src="'.$ofilename.'"/>
    </tr>    
  </table>';
  
    echo '
  <h3> <b>-></b> <a href="'.$ofilename.'" download> Download Image </a> <b><-</b> </h3>';
    
  } else if ($ftype === 'video') {
    
    // define parameters
    $ovfilename= "sc-".$fname;
    
    echo "<h1> Sorry </h1>";
    echo "<h1> Currently Under Maintenance... </h1>"; die();
    
    // define program name and command
    $program= "./GraphCut ";
    $filenname= "./".$fname;
    $command= $program.$filenname." ".$width." ".$height;
    
    echo $command;
    
    //die();
    
    // execute command   
    system($command);
    
    
    //die();
    
        echo '
  <h2> Seam Carved Results </h2>
  
  <table>
    <tr>
      <td> <h3> Original </h3>
    </tr>
    <tr>
      <td> <video controls> <source src="'.$fname.'"> </video>
    </tr>
    <tr>
      <td> <h3> Carved </h3>
    </tr>
    <tr>
      <td> <video controls> <source src="'.$ovfilename.'"> </video>
    </tr>    
  </table>';
  
    echo '
  <h3> <b>-></b> <a href="'.$ovfilename.'" download> Download Video </a> <b><-</b> </h3>';
    
  } else {
    
    echo "<h2> Under Construction </h2>";
    //sleep(1);
    header("Location: upload.php");
    die();
  
  }
  
} else {


  echo "<h2> invalid </h2>";
  sleep(2);
  header("Location: upload.php");
  die();
  
}

?>




</body>
  
</html>