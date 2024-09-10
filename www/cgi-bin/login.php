#!/usr/bin/php
<?php

function check_credentials($username, $password) {
    $expected_username = "admin";
    $expected_password = "admin";
    return $username === $expected_username && $password === $expected_password;
}

function displayPage($status, $username) {
    $message = $status;
    if ($status === "You have successfully logged in") {
        if ($username != null) {
            $message .= " as " . $username . ".";
        } else {
            $message .= ".";
        }
    }

    header("Content-Type: text/html; charset=UTF-8");

    echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login Status</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f4;
        }
        header {
            background-color: #333;
            color: #fff;
            padding: 1rem;
            text-align: center;
        }
        .wrapper {
            max-width: 800px;
            margin: 2rem auto;
            padding: 2rem;
            background-color: #fff;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        footer.footer {
            text-align: center;
            margin-top: 2rem;
            padding: 1rem;
            background-color: #333;
            color: #fff;
        }
        a.home-link {
            color: yellow;
            text-decoration: none;
        }
        a.home-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <header>
        <h1>Login Status</h1>
    </header>
    <div class="wrapper">
        <p>{$message}</p>
        <footer class="footer">
            <a href="/" class="home-link">Go to Index Page</a>
        </footer>
    </div>
</body>
</html>
HTML;
}

function main() {
    $query_string = getenv("QUERY_STRING") ?: "";
    $authenticated = getenv("AUTHENTICATED") ?: "";

    $login_success = "You have successfully logged in";
    $login_failed = "Login failed. Please check your credentials and try again.";
    $login_incomplete = "Username or password not provided.";
    $already_connected = "You are already connected.";

    if ($authenticated === "TRUE") {
        displayPage($already_connected, null);
        return;
    }

    $params = explode("&", $query_string);
    $username = null;
    $password = null;

    foreach ($params as $param) {
        if (strpos($param, "=") !== false) {
            list($key, $value) = explode("=", $param, 2);
            if ($key === "username") {
                $username = $value;
            } elseif ($key === "password") {
                $password = $value;
            }
        }
    }

    if ($username === null || $password === null) {
        displayPage($login_incomplete, null);
        return;
    } elseif (check_credentials($username, $password)) {
        displayPage($login_success, $username);
        return;
    } else {
        displayPage($login_failed, null);
        return;
    }
}

main();

?>
