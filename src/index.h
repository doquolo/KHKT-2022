const char mainpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Test</title>
    <style>
        :root {
            color-scheme: light dark;
        }
        .h1 {
            margin: 0rem 10rem 1rem 10rem;
        }
        .message-container {
            position: absolute;
            border: 1px solid black;
            display: flex;
            margin: 0rem 10rem 1rem 10rem;
            padding: 1rem;
            width: calc(100% - 20rem);
            height: calc(100% / 1.75);
            overflow: scroll;
            flex-direction: column;
            white-space: pre-line;
        }
        .typer {
            position: absolute;
            margin: 0rem 10rem 1rem 10rem;
            bottom: 1.5rem;
            width: calc(100% - 17.25rem);
        }
        .input {
            width: calc(100% - 6.75rem);
            border: 1px solid black;
        }
        .submit {
            border: 1px solid #000;
            border-left: none;
        }
        .message-container > div {
            padding: 0.5rem;
            border-radius: 0.45rem;
            width: auto;
            max-width: calc(100% / 2);
            border: 1px solid black;
            margin-bottom: 1rem;
        }
        .message-container > .sent {
            margin-left: auto;
        }
        .message-container > .received {
            margin-right: auto;
        }
        @media only screen and (max-width: 768px) {
            .h1 {
                margin: 0rem 1rem 1rem 1rem;
            }
            .message-container {
                margin: 0rem 1rem 1rem 1rem;
                width: calc(100% - 5rem);
            }
            .typer {
                bottom: 1rem;
                margin: 0rem 1rem 1rem 1rem;
                width: calc(100% - 2.265rem);
            }
        }
        </style>
        <script>
            const crypt = (salt, text) => {
                const textToChars = (text) => text.split("").map((c) => c.charCodeAt(0));
                const byteHex = (n) => ("0" + Number(n).toString(16)).substr(-2);
                const applySaltToChar = (code) => textToChars(salt).reduce((a, b) => a ^ b, code);
            
                return text
                .split("")
                .map(textToChars)
                .map(applySaltToChar)
                .map(byteHex)
                .join("");
            };
            
            const decrypt = (salt, encoded) => {
                const textToChars = (text) => text.split("").map((c) => c.charCodeAt(0));
                const applySaltToChar = (code) => textToChars(salt).reduce((a, b) => a ^ b, code);
                return encoded
                    .match(/.{1,2}/g)
                    .map((hex) => parseInt(hex, 16))
                    .map(applySaltToChar)
                    .map((charCode) => String.fromCharCode(charCode))
                    .join("");
            };
        </script>
</head>
<body>
    <div class="h1">
        <h1 style="margin-bottom: 0rem;">ESP softAP</h1><br/>
    </div>
    <div id="sent" class="message-container"></div>
    <div class="typer">
        <div>
            <input type="checkbox" id="enc" name="encrypt"><label for="encrypt">Encrypted?</label></input><br>
            <input type="checkbox" id="rly" name="relay"><label for="relay">Enable relaying?</label></input>
        </div>
        <div class="tab">
            <button class="tablink" id="message" onclick="showMessbox()">Message</button>
            <button class="tablink" id="gps" onclick="showGPSbox()">GPS</button>
        </div>
        <form id="form" onsubmit="return false;" class="message">
            <input type="text" id="inp" class="input"><input type="submit" class="submit" value="Send">
        </form>
        <div class="gps" style="display:none">
            <h5>Current GPS coordinate: <p id="gpsloc"></p> </h5>
            <button id="acquiregps" onclick="acquireGPS()">Acquire</button>
            <button id="sendgps" onclick="sendGPS()">Send</button>
        </div>
    </div>
    <script>
        const rlyCB =  document.querySelector('#rly');
        rlyCB.addEventListener('click', () => {
            // sending json
            var xhr = new XMLHttpRequest();
            var url = "/relay";
            xhr.open("POST", url, true);
            xhr.setRequestHeader("Content-Type", "application/json");
            const data = JSON.stringify({"relay": (rlyCB.checked) ? "1" : "0"});
            xhr.send(data);
        });
    </script> 
    <script>
        var pass;
        const encCB = document.querySelector('#enc');
        encCB.addEventListener('click', () => {
            if (encCB.checked) {
                var encpass = prompt("Enter encryption password: ");
                pass = encpass;
            }
        });
    </script>
    <script>
        const showMessbox = () => {
            document.querySelector("div.gps").style.display = "none";
            document.querySelector("form.message").style.display = "block";
        }
        const showGPSbox = () => {
            document.querySelector("div.gps").style.display = "block";
            document.querySelector("form.message").style.display = "none";
        }
    </script>
    <script>
        var name = prompt("Enter username");
        while (name == null || name == "") {
            name = prompt("Enter username before chatting!", "");
        }
        var divh1 = document.querySelector("div.h1");
        const divname = document.createElement("div");
        divname.innerHTML = `Chatting as <b>${name}</b>`;
        divh1.append(divname);  
    </script>
    <script>
        var currentgps;
        function calDistance(x1, y1, x2, y2) {
            return (Math.acos(Math.cos(x2-x1) - Math.cos(x1)*Math.cos(x2) + Math.cos(x1)*Math.cos(x2)*Math.cos(y2-y1)) * 6371);
        }
        const acquireGPS = () => {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = () => {
                if (xhr.readyState == XMLHttpRequest.DONE) {
                    try {
                        var gps = JSON.parse(xhr.responseText);
                    }
                    catch (err) {
                        return true;
                    }
                    currentgps = {lat:gps["lat"], long:gps["long"], sat:gps["sat"], time:gps["time"]};
                    document.querySelector("#gpsloc").innerText = `Latitude: ${currentgps["lat"]}N; Longitude: ${currentgps["long"]}E`;
                }   
            } 
            xhr.open('GET', "/gps", true);
            xhr.send(null);
        }
        const sendGPS = () => {
            var xhr = new XMLHttpRequest();
            var url = "/send";
            xhr.open("POST", url, true);
            xhr.setRequestHeader("Content-Type", "application/json");
            var content = `${currentgps["lat"]};${currentgps["long"]}`;
            // encrypt
            if (encCB.checked) {
                content = crypt(pass, content);
                var data = JSON.stringify({"m": content, "n": name, "t": "ge"});
            }
            else {
                var data = JSON.stringify({"m": content, "n": name, "t": "g!"});
            }
            xhr.send(data);
        }
    </script>
    <script>
        var temp;
        const updateMessage = () => {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = () => {
                if (xhr.readyState == XMLHttpRequest.DONE) {
                    // parse incoming json  
                    try {
                        var mess = JSON.parse(xhr.responseText);
                    }
                    catch (err) {
                        return true;
                    }
                    // check repeat
                    if (mess == temp || mess["m"] == "-1") return true;
                    temp = mess;
                    const div = document.createElement("div");
                    div.className = "received";
                    // decrypt
                    if (mess["e"] == "e") {
                        mess["m"] = decrypt(pass, mess["m"]);
                    }
                    // classify
                    switch (mess["t"]) {
                        case "m":
                            div.innerHTML = `<h5>${mess["n"]} (message)</h5>\n`;
                            div.innerText += mess["m"];
                            break;
                        case "g":
                            // gps coor form: latN;longE Ex: 12,05N;127,30E
                            var coor = mess["m"].split(";");
                            div.innerHTML = `<h5>${mess["n"]} (gps)</h5>\n`;
                            div.innerText += `${coor[0]}N;${coor[1]}E (${Math.round(calDistance(currentgps[lat], currentgps[lon], coor[0], coor[1]))}km from us)`;
                            break;
                    }
                    document.querySelector("#sent").append(div);
                }
            } 
            xhr.open('GET', "/update", true);
            xhr.send(null);
        }
        // setInterval(updateMessage, 500);
    </script>
    <script>
        const form = document.querySelector('#form');
        form.addEventListener("submit", ()=>{
            var text = document.querySelector("#inp").value;
            console.log(text);
            if (text.length >= 50) {alert("Content must be less than 50 characters!");return false;}
            if (text == "") {alert("Content Empty!");return false;}
            document.querySelector("#inp").value = "";
            const div = document.createElement("div");
            div.innerText = text;
            div.className = "sent";
            document.querySelector("#sent").append(div);
            // sending json
            var xhr = new XMLHttpRequest();
            var url = "/send";
            xhr.open("POST", url, true);
            xhr.setRequestHeader("Content-Type", "application/json");
            // encrypt
            if (encCB.checked) {
                text = crypt(pass, text);
                var data = JSON.stringify({"m": text, "n": name, "t": "me"});
            }
            else {
                var data = JSON.stringify({"m": text, "n": name, "t": "m!"});
            }
            xhr.send(data);
        });
    </script>
</body>
</html>
)=====";