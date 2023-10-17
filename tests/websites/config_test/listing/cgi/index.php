<?php
$folder = '.'; 

// Check if the folder exists
if (!is_dir($folder)) {
    echo "Folder not found: $folder";
    exit;
}

// Open the directory
if ($handle = opendir($folder)) {
    echo "<h2>Files in $folder:</h2>";
    echo "<table style='border-collapse: collapse; width: 100%;'>";
    echo "<tr><th style='border: 1px solid black; padding: 8px;'>File</th><th style='border: 1px solid black; padding: 8px;'>Last Modified</th><th style='border: 1px solid black; padding: 8px;'>File Size</th></tr>";
    // Loop through the directory
    while (false !== ($file = readdir($handle))) {
        // Exclude "." and ".." entries
        if ($file != "." && $file != "..") {
            $filePath = $folder . '/' . $file;
            $fileSize = filesize($filePath);
            $lastModified = date("Y-m-d H:i:s", filemtime($filePath));
            
            echo "<tr>";
            echo "<td style='border: 1px solid black; padding: 8px;'><a href='$folder/$file'>$file</a></td>";
            echo "<td style='border: 1px solid black; padding: 8px;'>$lastModified</td>";
            echo "<td style='border: 1px solid black; padding: 8px;'>$fileSize bytes</td>";
            echo "</tr>";
        }
    }
    echo "</table>";
    // Close the directory
    closedir($handle);
} else {
    echo "Could not open the directory: $folder";
}
?>
