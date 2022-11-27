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
        .message-container > div {
            padding: 0.5rem;
            border-radius: 0.45rem;
            width: auto;
            max-width: calc(100% / 2);
            border: 1px solid black;
            margin-bottom: 1rem;
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
        <h1 style="margin-bottom: 0rem;">Control Panel</h1><br/>
    </div>
    <div id="sent" class="message-container"></div>
    </div>
    <script>
        var name = prompt("Enter device name");
        while (name == null || name == "") {
            name = prompt("Enter device name!", "");
        }
        var encpass = prompt("Enter encryption password: ");
        pass = encpass;
        var divh1 = document.querySelector("div.h1");
        const divname = document.createElement("div");
        divname.innerHTML = `Hi <b>${name}</b>`;
        divh1.append(divname);  
    </script>
    <script>
        const cmd = [];
        var temp;
        const handlecmd = (b) => {
            cmd.push(b);
        }
        const sendback = (a) => {
            if (cmd.length == 0) return 0;
            var xhr1 = new XMLHttpRequest();
            var url1 = "/control";
            xhr1.open("POST", url1, true);
            xhr1.setRequestHeader("Content-Type", "application/json");
            var data1 = JSON.stringify({"c": cmd.pop()});
            xhr1.send(data1);
        }
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
                    if (mess["t"] == "c") {
                        div.innerHTML = `<h5>${mess["n"]} (command)</h5>\n`;
                        div.innerText += mess["m"];
                        // sending back decrypted message
                        handlecmd(mess["m"]);
                       }
                    document.querySelector("#sent").append(div);
                }
            } 
            xhr.open('GET', "/update", true);
            xhr.send(null);
        }
        const update = setInterval(function () {
            updateMessage();
            sendback();
        }, 500);
    </script>
</body>
</html>
)=====";
