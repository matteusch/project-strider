const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<body>
    <canvas id="cam" width="160" height="120" style="border:1px solid black;"></canvas>
    <script>
        const canvas = document.getElementById('cam');
        const ctx = canvas.getContext('2d');
        
        const esp_ip = "/stream";

        async function fetchFrame() {
            try {
                const response = await fetch(esp_ip);
                const buffer = await response.arrayBuffer();
                const bytes = new Uint8Array(buffer);
                
                const imgData = ctx.createImageData(canvas.width, canvas.height);
                
                for (let i = 0; i < bytes.length; i++) {
                    const val = bytes[i];
                    imgData.data[i * 4] = val;     // R
                    imgData.data[i * 4 + 1] = val; // G
                    imgData.data[i * 4 + 2] = val; // B
                    imgData.data[i * 4 + 3] = 255; // A
                }
                
                ctx.putImageData(imgData, 0, 0);
            } catch (err) {
                console.error("Błąd pobierania:", err);
            }
            setTimeout(fetchFrame, 100);
        }

        fetchFrame();
    </script>
</body>
</html>
)rawliteral";