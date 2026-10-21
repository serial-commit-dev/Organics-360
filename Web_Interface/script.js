function updateClock() {
  const now = new Date();
  const options = {
    year: 'numeric', month: 'short', day: 'numeric',
    hour: '2-digit', minute: '2-digit', second: '2-digit'
  };
  document.getElementById('live-clock').innerText = now.toLocaleDateString('en-US', options);
}

setInterval(updateClock, 1000);
updateClock();

async function fetchGasData() {
    try {
        const response = await fetch('http://192.168.4.1/api/data');
        const data = await response.json();
        document.getElementById('sensor-val').innerText = data.sensor_value;
        document.getElementById('gas-status').innerText = data.gas_status;
    } catch (error) {
        console.error("Error fetching data from ESP32:", error);
    }
}

setInterval(fetchGasData, 1000);
