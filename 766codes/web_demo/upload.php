<!DOCTYPE html>
<html>

<head>
  <title> seam carving demo </title>
  <style type='text/css'>
    a:link { color: #000fff; }
    a:visited { color: #000fff; }
    a:hover { color: #0099ff; }
    a:active { color: #000fff; }
  </style> 
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js"> </script>
  <script type="text/javascript"> </script>
</head>

<body bgcolor="#FFFFF5" >
<font face="Comic sans MS">

  <h1> Seam Carving Demo</h1>
  
  <h3> select a video / image file to upload </h3>

  <form action="upload.php" method="POST" enctype="multipart/form-data">
    <table>
      <tr>
        <td> 
          <input type="file" name="file" value="Choose a file" accept="image/*, video/*" />
        <td> 
          <input type="submit" value="Upload file" />
        <td> 
          <input type="submit" value="Reset" />  
      </tr>
    </table>    
  </form>
  <br>
  
<?php
  //phpinfo();
  // define parameters
  $location= 'uploads/';  
  $error= $_FILES['file']['error'];  
  $name= $_FILES['file']['name'];  $size= $_FILES['file']['size'];
  $type= $_FILES['file']['type'];  $temp_name= $_FILES['file']['tmp_name'];
  // check if file is larger than 50MB
  if ($size >= 52428800) {
    echo "file is larger than 50MB"." <br>";
    die();
  }
  // if name is set
  if (isset($name)) {
    $load= 0; 
    if (!empty($name)) {
      // if move successfully
      if (move_uploaded_file($temp_name,$name)) {
        chmod($name, 0777); 
        $mb= number_format($size/1024/1024,2,'.','');
        $kb= number_format($size/1024,2,'.','');
        $ftypes = explode("/", $type);
        $ftype= $ftypes[0];
        echo "uploaded successfully! <br>".$ftype." file=<";
        if ($mb >= 1.0) {
          echo $mb."MB>";
        } else {
          echo $kb."KB>";
        }
        echo "<br>";
        $load= 1;
      }
    } else {
      echo "please select a file"." <br>";
    }
    // display file 
    if ($load === 1) {
      if ($ftype === 'image') {
        // get image dimension
        list($imgwidth,$imgheight,$imgtype,$imgattr) = getimagesize($name);
        // read image
        echo 'name= '.$name.', dimension=<'.$imgwidth.'x'.$imgheight.'> <br>';
        echo '<img src="'.$name.'"/>';
      } else if ($ftype === 'video') {
        // read video
        echo '
  <video id="vid" controls> <source src="'.$name.'"> </video>
  <br> <br>
  <table>
    <tr>
      <td> dimension[w,h]= </td>
      <td id="output1"> </td>
      <td> x </td>
      <td id="output2"> </td> 
    </tr> 
  </table>'; 
        echo "
  <script> 
    $(document).ready(function(){ 
      $('#vid').on('loadedmetadata',function(){
        document.getElementById('output1').innerHTML= this.videoWidth;
        document.getElementById('output2').innerHTML= this.videoHeight; 
      });
    }); 
  </script>";
       } else {
         echo "file is not image or video type <br>";
         die();
       } 
    }
    // prompt user to select size
    if ($load === 1) {
      echo '
  <h3> select desired dimension </h3>';
  	  echo '
  Specific the output dimension in the boxes below. <br>  
  <a href="size_ref.html" target="_blank">Open Dimension Reference</a> 
  to look for desired dimension <br> <br> '; 
  	  // begin form
  	  echo '
  <form id="select_form" action="results.php" method="POST" name="select_form" target="_blank">
    <table>
      <tr>
        <td> file type
        <td> <input type="text" name="select_filetype" value="'.$ftype.'" readonly />
      </tr>
      <tr>
        <td> file name
        <td> <input type="text" name="select_filename" value="'.$name.'" readonly />
      </tr>
      <tr> 
        <td> output width
        <td> <input type="number" name="select_width" min="1" max="1500" value="';
      if ($ftype === 'image') { echo $imgwidth; } 
      echo '"id="select_width" required/> * max 1500
      </tr>
      <tr>
        <td> output height
        <td> <input type="number" name="select_height" min="1" max="1500" value="';
      if ($ftype === 'image') { echo $imgheight; } 
      echo '"id="select_width" required/> * max 1500
      </tr>
      <tr>';
      if ($ftype === 'image') {           
        echo '
        <td> select method
        <td>
          <input type="radio" name="select_method" value="dp" checked>Dynamic Programming <br>
          <input type="radio" name="select_method" value="gc">Graph Cut <br> 
      </tr>
      <tr>
        <td> select energy
        <td>    
          <input type="radio" name="select_energy" value="b" checked>Backward <br>
          <input type="radio" name="select_energy" value="f">Forward <br>';
      } else if ($ftype === 'video') { 
        echo '
        <td> method
        <td> Graph Cut <br> 
      </tr>
      <tr>
        <td> energy
        <td> Forward <br>';
    }
      echo '
        </tr>    
      </table>  
    <input type="submit" value="choose the selected options" id="dbutton">
  </form>  
  <h2> Warning: After clicking submit, please wait for a few minutes as the carving process is running </h2>';
    $load= 0;
  } 
}  
  
 
?>

</body>
</html>
