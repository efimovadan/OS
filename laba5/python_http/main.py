from fastapi import FastAPI, HTTPException
from fastapi.responses import HTMLResponse
import requests
import json
from datetime import datetime

app = FastAPI()

# URL C++ сервера
CPP_SERVER_URL = "http://localhost:8080"

# Функция для парсинга ответа сервера
def parse_server_response(response_text: str) -> dict:
    try:
        data = json.loads(response_text)
        if isinstance(data, list):
            # Если это список, возвращаем его как есть
            return data
        else:
            # Если это объект, преобразуем timestamp в UNIX время
            data['timestamp'] = datetime.strptime(data['timestamp'], "%Y-%m-%d %H:%M:%S").timestamp()
            return data
    except Exception as e:
        raise ValueError(f"Error parsing response: {e}")

# Функция для преобразования температуры и времени в HTML таблицу
def generate_html_table(data: dict) -> str:
    table_rows = "".join(
        f"<tr><td>{datetime.fromtimestamp(data['current']['timestamp']).strftime('%Y-%m-%d %H:%M:%S')}</td>"
        f"<td>{data['current']['temperature']:.2f}°C</td>"
        f"<td>{data['hourly']['average']:.2f}°C</td>"
        f"<td>{data['daily']['average']:.2f}°C</td></tr>"
    )
    return f"""
    <html>
    <head>
        <title>Temperature Data</title>
        <style>
            table {{
                width: 50%;
                border-collapse: collapse;
                margin: 20px auto;
            }}
            th, td {{
                border: 1px solid #ddd;
                padding: 8px;
                text-align: center;
            }}
            th {{
                background-color: #f4f4f4;
            }}
            tr:hover {{
                background-color: #f1f1f1;
            }}
        </style>
    </head>
    <body>
        <h2 style="text-align: center;">Temperature Data</h2>
        <table>
            <thead>
                <tr>
                    <th>Timestamp</th>
                    <th>Current Temperature</th>
                    <th>Hourly Average Temperature</th>
                    <th>Daily Average Temperature</th>
                </tr>
            </thead>
            <tbody>
                {table_rows}
            </tbody>
        </table>
    </body>
    </html>
    """

@app.get("/", response_class=HTMLResponse)
def index():
    return """
    <html>
    <head>
        <title>Temperature Data Viewer</title>
    </head>
    <body>
        <h1 style="text-align: center;">Temperature Data Viewer</h1>
        <div style="text-align: center;">
            <a href="/temperature_data">Current and Average Temperatures</a><br>
            <a href="/all_temperatures">All Temperatures</a>
        </div>
    </body>
    </html>
    """

@app.get("/temperature_data", response_class=HTMLResponse)
def get_temperature_data():
    try:
        current_response = requests.get(f"{CPP_SERVER_URL}/current_temperature")
        current_response.raise_for_status()
        current_data = parse_server_response(current_response.text)

        hourly_response = requests.get(f"{CPP_SERVER_URL}/hourly_avg")
        hourly_response.raise_for_status()
        hourly_data = parse_server_response(hourly_response.text)

        daily_response = requests.get(f"{CPP_SERVER_URL}/daily_avg")
        daily_response.raise_for_status()
        daily_data = parse_server_response(daily_response.text)

        data = {
            "current": current_data,
            "hourly": hourly_data,
            "daily": daily_data
        }

        html = generate_html_table(data)
        return HTMLResponse(content=html)
    except (requests.RequestException, ValueError) as e:
        raise HTTPException(status_code=500, detail=f"Error fetching data: {e}")

@app.get("/all_temperatures", response_class=HTMLResponse)
def get_all_temperatures():
    try:
        response = requests.get(f"{CPP_SERVER_URL}/all_temperatures")
        response.raise_for_status()
        data = parse_server_response(response.text)

        table_rows = "".join(
            f"<tr><td>{datetime.fromtimestamp(datetime.strptime(item['timestamp'], '%Y-%m-%d %H:%M:%S').timestamp()).strftime('%Y-%m-%d %H:%M:%S')}</td>"
            f"<td>{item['temperature']:.2f}°C</td></tr>"
            for item in data
        )

        html = f"""
        <html>
        <head>
            <title>All Temperatures</title>
            <style>
                table {{
                    width: 50%;
                    border-collapse: collapse;
                    margin: 20px auto;
                }}
                th, td {{
                    border: 1px solid #ddd;
                    padding: 8px;
                    text-align: center;
                }}
                th {{
                    background-color: #f4f4f4;
                }}
                tr:hover {{
                    background-color: #f1f1f1;
                }}
            </style>
        </head>
        <body>
            <h2 style="text-align: center;">All Temperatures</h2>
            <table>
                <thead>
                    <tr>
                        <th>Timestamp</th>
                        <th>Temperature</th>
                    </tr>
                </thead>
                <tbody>
                    {table_rows}
                </tbody>
            </table>
        </body>
        </html>
        """
        return HTMLResponse(content=html)
    except (requests.RequestException, ValueError) as e:
        raise HTTPException(status_code=500, detail=f"Error fetching data: {e}")