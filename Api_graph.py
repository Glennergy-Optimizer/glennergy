from flask import Flask, send_file, jsonify
import requests
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import io

app = Flask(__name__)
C_API_URL = "http://localhost:8080/api/results"

@app.route('/graph/<int:home_id>')
def get_graph(home_id):
    try:
        # Fetch data from C API
        resp = requests.get(f"{C_API_URL}/{home_id}", timeout=5)
        if resp.status_code != 200:
            return jsonify({"error": f"API returned {resp.status_code}"}), 500
        
        data = resp.json()
        
        # Extract plotting data
        slots = data.get('slots', [])
        if not slots:
            return jsonify({"error": "No slot data available"}), 404
        
        # Parse timestamps from API and extract values
        timestamps = []
        solar = []
        prices = []
        
        for slot in slots:
            # Parse ISO 8601 timestamp
            ts_str = slot.get('timestamp', '')
            if ts_str:
                ts = datetime.fromisoformat(ts_str)
                timestamps.append(ts)
                solar.append(slot.get('solar_kwh', 0))
                prices.append(slot.get('grid_price', 0))
        
        if not timestamps:
            return jsonify({"error": "No valid timestamps in data"}), 500
        
        num_slots = len(timestamps)
        
        # Create dual-axis plot
        fig, ax1 = plt.subplots(figsize=(16, 7))
        
        # Solar production (bars)
        ax1.bar(timestamps, solar, width=0.01, alpha=0.6, color='orange', label='Solar Production')
        ax1.set_xlabel('Time', fontsize=12)
        ax1.set_ylabel('Solar Production (kWh)', color='orange', fontsize=12)
        ax1.tick_params(axis='y', labelcolor='orange')
        ax1.grid(True, alpha=0.3)
        
        # Format x-axis to show time
        ax1.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
        ax1.xaxis.set_major_locator(mdates.HourLocator(interval=2))
        plt.xticks(rotation=45, ha='right')
        
        # Grid price (line)
        ax2 = ax1.twinx()
        ax2.plot(timestamps, prices, color='blue', linewidth=2, label='Grid Price', marker='.')
        ax2.set_ylabel('Grid Price (SEK/kWh)', color='blue', fontsize=12)
        ax2.tick_params(axis='y', labelcolor='blue')
        
        # Title and legend
        total_solar = data.get('total_solar_kwh', sum(solar))
        avg_price = data.get('avg_grid_price', sum(prices)/len(prices) if prices else 0)
        date_str = timestamps[0].strftime('%Y-%m-%d')
        
        plt.title(f'Solar Production vs Grid Price - Home {home_id} ({date_str})\n'
                  f'Total Solar: {total_solar:.2f} kWh | '
                  f'Avg Price: {avg_price:.3f} SEK/kWh | '
                  f'Slots: {num_slots}', 
                  fontsize=14)
        
        # Combine legends
        lines1, labels1 = ax1.get_legend_handles_labels()
        lines2, labels2 = ax2.get_legend_handles_labels()
        ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left')
        
        plt.tight_layout()
        
        # Save to bytes buffer
        img = io.BytesIO()
        plt.savefig(img, format='png', dpi=100, bbox_inches='tight')
        img.seek(0)
        plt.close()
        
        return send_file(img, mimetype='image/png')
        
    except requests.exceptions.RequestException as e:
        return jsonify({"error": f"Failed to connect to C API: {str(e)}"}), 500
    except Exception as e:
        return jsonify({"error": f"Internal error: {str(e)}"}), 500

@app.route('/health')
def health():
    return jsonify({"status": "ok", "service": "graph-api"})

if __name__ == '__main__':
    print("Starting Graph API on http://localhost:5000")
    print("Example: http://localhost:5000/graph/1")
    app.run(host='0.0.0.0', port=5000, debug=True)