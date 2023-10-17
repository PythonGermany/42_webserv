<?php
header('Content-Type: text/plain');
echo "_SERVER:\r\n";
var_dump($_SERVER);
echo "_GET:\r\n";
var_dump($_GET);
echo "_POST:\r\n";
var_dump($_POST);
echo "_FILES:\r\n";
var_dump($_FILES);
echo "_REQUEST:\r\n";
var_dump($_REQUEST);
echo "_SESSION:\r\n";
var_dump($_SESSION);
echo "_ENV:\r\n";
var_dump($_ENV);
echo "_COOKIE:\r\n";
var_dump($_COOKIE);
?>
