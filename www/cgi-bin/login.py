#!/usr/bin/env python3
import os

def print_html_page(status, username):
    message = status;
    if (status == "You have successfully logged in"):
        if (username != ""):
            message += " as " + username + ".";
        else:
            message += ".";

    print("""
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
        <p>{{}}</p>
        <footer class="footer">
            <a href="/" class="home-link">Go to Index Page</a>
        </footer>
    </div>
</body>
</html>
    """.format(message))

def check_credentials(username, password):
    expected_username = "admin"
    expected_password = "admin"
    return username == expected_username and password == expected_password

def main():
    query_string = os.getenv("QUERY_STRING", "")
    authenticated = os.getenv("AUTHENTICATED", "")

    login_success = "You have successfully logged in"
    login_failed = "Login failed. Please check your credentials and try again."
    login_incomplete = "Username or password not provided."
    already_connected = "You are already connected."

    if authenticated == "TRUE":
        print_html_page(already_connected, "")
        return
    
    params = query_string.split("&")
    username = None
    password = None
    
    for param in params:
        if "=" in param:
            key, value = param.split("=", 1)
            if key == "username":
                username = value
            elif key == "password":
                password = value
                

    if username is None or password is None:
        print_html_page(login_incomplete, "")
        return
    elif check_credentials(username, password):
        print_html_page(login_success, username)
        return
    else:
        print_html_page(login_failed, "")
        return

if __name__ == "__main__":
    main()
