#!/usr/bin/env python3
import os

def main():
    query_string = os.getenv("QUERY_STRING", "")
    message = None
    key, value = query_string.split("=", 1)
    message = value

    print("status=200")
    print("")

    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Test</title>
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
        <h1>CGI Script Test</h1>
    </header>
    <div class="wrapper">
        <p>Your message:</p>
        <p>""")
    print(message)
    print("""</p>
        <footer class="footer">
            <a href="/" class="home-link">Go to Index Page</a>
        </footer>
    </div>
</body>
</html>""")

if __name__ == "__main__":
    main()