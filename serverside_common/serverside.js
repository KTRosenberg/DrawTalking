"use strict";

const net = require('net');
const fs = require('fs');
const {spawn, child} = require('child_process');
const readline = require('readline');

const http = require('http');

const os = require('os');

const vm = require('vm');
let localContext = {};
vm.createContext(localContext);

const util = require('util');

const chokidar  = require('chokidar');

const pathModule = require('path');

let enableReloading = true;

{
    process.argv.forEach((val, index) => {
        val = val.trim();
        const info = val.split("=");
        if (info.length == 2) {
            console.log(info[0], "=", info[1]);
            try {
                if (info[0] == 'reload') {
                    info[1] = info[1].toLowerCase();
                    if (info[1] == 'false' || info[1] == 'f' || info[1] == '0') {
                        enableReloading = false;
                    }
                }
            } catch (err) {
                console.error(err);
            }
        }
    });
}
console.log("reload enabled=[", enableReloading, "]");

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
})


let timeMSBegin = Date.now();

let dynamicLibraryVersion = 0;

function readFile(filePath) {
    if (filePath.startsWith('~/') || filePath === '~') {
        filePath = filePath.replace('~', os.homedir());
    }
    return new Promise((resolve, reject) => {
        fs.readFile(filePath, 'utf-8', function (error, data) {
            if (error) return reject(error);

            console.log("loading", filePath);

            resolve(data);
        });
    });
}

function writeFile(filePath, data) {
    if (filePath.startsWith('~/') || filePath === '~') {
        filePath = filePath.replace('~', os.homedir());
    }
    return new Promise((resolve, reject) => {
        fs.writeFile(filePath, data, 'utf-8', function (error) {
            if (error) return reject(error);

            resolve(true);
        });
    });
}

let projectFilePathAbsolute = ""
{
    let P = process.cwd().split(pathModule.sep)
    P.pop()
    projectFilePathAbsolute = pathModule.join(P.join(pathModule.sep), "apple_platforms", "Make The Thing") + pathModule.sep;
    console.log(projectFilePathAbsolute)
}

function main() {

function isJSONString(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }

    return true;
}


const speechIDToData = new Map();
const speechIDList = [];

// main program
let pyProgMain = null;
// optional intermediate step
let pyProgAux = null;

// program to send to in first step,
// if not pyProgMain, stdout should be redirected to pyProgMain
let pyProgTo = null;

const connections = [];
const cmdConnections = [];

let loadInProgress = false;
function dynamicLoad(connections) {
    if (!enableReloading) {
        return;
    } else if (loadInProgress) {
        return;
    }
    loadInProgress = true;


    dynamicLibraryVersion = 1 + (Date.now() - timeMSBegin);

    let dynamicLibraryVersionString = dynamicLibraryVersion + "";
    let build = spawn('zsh', ["./dyld.zsh", dynamicLibraryVersionString]);
    build.stdin.setEncoding('utf-8');
    build.stdout.setEncoding('utf-8');


    build.stdout.on('data', (data) => {
        console.log(`stdout: ${data}\n`,);
    });

    build.stderr.on('data', (data) => {
        console.error(`stderr: ${data}\n`);
    });

    build.on('error', (error) => {
        loadInProgress = false;
        console.error(`error: ${error.message}`);
    });

    build.on('close', (code) => {
        console.log(`child process exited with code ${code}`);
        if (code != 0) {
            loadInProgress = false;
            return;
        }
        for (let i = 0; i < connections.length; i += 1) {
            console.log("MSG: dyl:" + dynamicLibraryVersionString + '\r\n');
            connections[i].write("dyl:" + dynamicLibraryVersionString + '\r\n');
        }
        loadInProgress = false;
    }); 
}

let watcher = null;
if (enableReloading) {
    watcher = chokidar.watch([
        projectFilePathAbsolute + "dynamic_main.cpp",
        projectFilePathAbsolute + "dynamic_main.hpp",
        projectFilePathAbsolute + "shader_playground.hpp"
        ], {
        ignored: /(^|[\/\\])\../, // ignore dotfiles
        persistent: true
    });

    watcher
    .on('add', path => console.log(`File ${path} has been added`))
    .on('change', path => {
        console.log(`File ${path} has been changed`);

        dynamicLoad(connections);
    })
    .on('unlink', path => console.log(`File ${path} has been removed`))
    .on('addDir', path => console.log(`Directory ${path} has been added`))
    .on('unlinkDir', path => console.log(`Directory ${path} has been removed`))
    .on('error', error => console.log(`Watcher error: ${error}`))
    .on('ready', () => console.log('Initial scan complete. Ready for changes'))
    .on('close', () => console.log('Watcher closed'))
    // .on('raw', (event, path, details) => { // internal
    //     console.log('Raw event info:', event, path, details);
    // });

    console.log('watched', watcher.getWatched());
}

const logCommands = false

async function loadScriptsConfig() {

    try {
        const SCRIPT_CONFIG_DATA = await readFile("./script_extern_config.json")
        return JSON.parse(SCRIPT_CONFIG_DATA);
    } catch (ex) {
        const DEFAULT_EXTERNAL_SCRIPT_CONFIG = {
            "isExternalEnabled" : false,
            "environmentSetup" : "source py-ext/bin/activate",
            "command" : ["python3"],
            "entryScript" : "./script-language-processing-extern.py"
        };
        console.log("writing default external script config")
        await writeFile("script_extern_config.json", JSON.stringify(DEFAULT_EXTERNAL_SCRIPT_CONFIG, null, "    ") + "\n");
        return DEFAULT_EXTERNAL_SCRIPT_CONFIG;
    }
}

async function initLingua() {
    

    pyProgMain = spawn('python3', ['./lingua.py']).on('error', function( err ) { throw err });
    pyProgMain.stdin.setEncoding('utf-8');
    pyProgTo = pyProgMain;

    const LINGUA_PROCESS_INTERVAL_INIT   = 1000;
    const LINGUA_PROCESS_INTERVAL_ACTIVE = 0x000;
    let isInit = false;
    let intervalID = setInterval(
        function() {
            console.log("initializing Lingua ...");
            pyProgMain.stdin.write('');
        }, 
        LINGUA_PROCESS_INTERVAL_INIT
    );

    let dataHdr = {
        isStart       : true,
        lengthArrived : 0,
        length        : 0,
        chunks        : []
    };


    function handleData(data) {
        if (data == undefined || data == null) {
            return;
        }

        
        for (let i = 0; i < connections.length; i += 1) {
            connections[i].write(data); // '\r\n' is the end-message marker
        }
    }


    pyProgMain.stdout.on('data', function(data) {
        if (!isInit) {
            
            try {
                const parse = JSON.parse(data)['msg']
                if (logCommands) {
                    console.log("[", parse, "]");
                }
                if (parse == "LINGUA_INITIALIZATION_FAILED") {
                    console.error("Lingua initialization was unsuccessful")
                    pyProgMain.stdout.on('data', function(data) {});
                    clearInterval(intervalID);
                    onExit()
                    return;  
                }
            } catch (ex) {
                console.log(data.toString())
                return;
            }

            loadScriptsConfig().then((conf) => {
                clearInterval(intervalID);
                
                // by default, external extensions are ignored
                if (conf.isExternalEnabled) {
                    console.log("Auxiliary Extension Config (Enabled):", conf);
                    let pyAuxCommand = conf.environmentSetup + " && " + conf.command + " " + conf.entryScript;
                    console.log("Starting external processing step", pyAuxCommand);
                    pyProgAux = spawn(pyAuxCommand, [], {"shell" : true}).on('error', function( err ) { throw err });
                    pyProgAux.stdin.setEncoding('utf-8');

                    pyProgAux.stdout.pipe(pyProgMain.stdin).on("error", (err) => { 
                        console.error(err);
                        pyProgAux.kill("SIGINT");
                        pyProgAux.kill();
                    }).on("close", () => { 
                        pyProgAux.kill("SIGINT");
                        pyProgAux.kill(); 
                    });

                    // send text to process in auxiliary script
                    pyProgTo = pyProgAux;

                    pyProgAux.stderr.on('data', function(data) {
                        console.error(data.toString(), "\n");
                    });
                    pyProgAux.on('exit', () => {
                        pyProgAux.kill();
                    })
                } else {
                    console.log("Auxiliary Extension Config Default (Disabled):", conf);
                    console.log("\nReady!");
                }


                function rlOnAnswer(answer) {
                    const args = answer.split(/[ ,]+/);

                    switch (args.length) {
                    case 1: {
                        const cmd = args[0].toLowerCase();
                        switch (cmd) {
                        case "conf":
                            const CONF_PATH = "~/Documents/Make The Thing-data/config/init.json";
                            readFile(CONF_PATH).then((data) => {
                                console.log(data);

                                if (!isJSONString(data)) {
                                    console.error("invalid file");
                                    return;
                                }

                                for (let i = 0; i < connections.length; i += 1) {
                                    // load file
                                    connections[i].write("ldf:" + CONF_PATH + "|" + data + '\r\n'); // '\r\n' is the end-message marker
                                }

                                localContext = JSON.parse(data);
                                vm.createContext(localContext);

                                
                            }).catch((err) => {
                                console.error(err);
                            });
                            break;
                        case "load":
                        case "ld":
                        case "l": {
                            readFile(DEFAULT_TEMPLATE_PATH).then((data) => {
                                console.log(data);

                                let regex = /\,(?=\s*?[\}\]])/g;
                                data = data.replace(regex, ''); // remove all trailing commas

                                if (!isJSONString(data)) {
                                    console.error("invalid file");
                                    return;
                                }

                                for (let i = 0; i < connections.length; i += 1) {
                                    // load file
                                    connections[i].write("ldf:" + DEFAULT_TEMPLATE_PATH + "|" + data + '\r\n'); // '\r\n' is the end-message marker
                                }
                            }).catch((err) => {
                                console.error(err);
                            });
                            break;
                        }
                        case "dyl": {
                            dynamicLoad(connections);
                        
                            break;
                        }
                        }
                        break;
                    }
                    case 2: {
                        const cmd = args[0].toLowerCase();
                        switch (cmd) {
                        case "load":
                        case "ld":
                        case "l": {
                            const path = TEMPLATES_PATH + args[1];
                            readFile(path).then((data) => {
                                console.log(data);

                                let regex = /\,(?=\s*?[\}\]])/g;
                                data = data.replace(regex, ''); // remove all trailing commas

                                if (!isJSONString(data)) {
                                    console.error("invalid file");
                                    return;
                                }

                                for (let i = 0; i < connections.length; i += 1) {
                                    // load file
                                    connections[i].write("ldf:" + path + "|" + data + '\r\n'); // '\r\n' is the end-message marker
                                }
                                
                            }).catch((err) => {
                                console.error(err);
                            });
                        }
                        }
                        break;
                    }
                    default: {
                        console.log("local context command");
                        try {
                            vm.runInContext(answer, localContext);
                            console.log(localContext);
                            for (let i = 0; i < connections.length; i += 1) {
                                connections[i].write("ldf:.|" + JSON.stringify(localContext) + '\r\n'); // '\r\n' is the end-message marker
                            }
                        } catch (err) {
                            console.error(err);
                        }
                        break;
                    }
                    }

                    rl.question('', rlOnAnswer);
                }
                rl.question('', rlOnAnswer);

            });

            isInit = true;

            return;
        }

        handleData(data);
    });
    pyProgMain.stderr.on('data', function(data) {
        console.error(data.toString(), "\n");
    });
    pyProgMain.on('close', () => {
        //console.log("lingua closed");
    });
}
function linguaWrite(idTextPair) {
    //console.log("writing to Python Lingua");
    pyProgTo.stdin.write(idTextPair + '\n');
}


const questionWords = new Set([
    'will', "won't", "didn't", 'should', "would",
    "wouldn't", 'could', "couldn't", "haven't", 'who', 'why', 
    "why's", "aren't", "ain't", 'how', 'whose', 
    "what's", "what is", "what are", "what're", "who's", "how's", 
    "when's", "when is", "when are", "does", "doesn't",
    "for whom", "for what", "for which", "for whose",
    "have", "did", "is", "isn't", 'was', "wasn't", "are",
    "wie", "wer", "wem", "wann", "wohin", "woher", "wo", "warum", "was", "welche",
]);

const TEMPLATES_PATH = "./templates";
const DEFAULT_TEMPLATE_PATH = TEMPLATES_PATH + "/default.json";
// plain TCP server
const END_MSG_TOKEN = "\r\n";
const CMD_LENGTH = 4;

let latestID = -1;
let latestModCount = -1;

function handleSpeechInput(cmd, mode) {
    const payload = cmd.substring(CMD_LENGTH);

    const idSepIdx = payload.indexOf("&");
    const modCountSepIdx = payload.indexOf(":");

    const id = parseInt(payload.substring(0, idSepIdx));
    const modCount = parseInt(payload.substring(idSepIdx + 1, modCountSepIdx));
    if (logCommands) {
        console.log("incoming id %d incoming modCount %d latestID %d latestModCount %d\n", id, modCount, latestID, latestModCount);
    }
    if (id != 0) {
        if (id < latestID && id > -1) {
            return;
            // cancel
        } else if (modCount < latestModCount && id > -1) {
            return;
            // cancel
        }
    }

    let msg = payload.substring(modCountSepIdx + 1).trim()
    
    {

        msg = msg.trim()
        let data = msg//data.trim()

        const msgLastChar = msg.charAt(msg.length - 1);
        const dataLastChar = data.charAt(data.length - 1);

        ////console.log('msglast=[' + msgLastChar + "] datalast=[" + dataLastChar + "]");
        // if (msgLastChar != ',' &&
        //     dataLastChar == ',') {

        //     data = data.substring(0, data.length - 1);
        // } else if (msgLastChar != ':' &&
        //            dataLastChar == ':') {
        //     data = data.substring(0, data.length - 1);
        // } else if (msgLastChar != ';' &&
        //            dataLastChar == ';') {
        //     data = data.substring(0, data.length - 1);
        // } else if (msgLastChar != '-' &&
        //            dataLastChar == '-') {
        //     data = data.substring(0, data.length - 1).trim();
        // }

        if (msgLastChar == '.' || msgLastChar == '?') {
            msg = data;
        } else if (dataLastChar != '.' && dataLastChar != '?') {
            let qtest = data;
            //qtest = qtest.replace(':', ' : ');
            //qtest = qtest.replace(';', ' ; ');
            qtest = qtest.replace(/\s{2,}/g, ' ');
            const words = qtest.split(' ');


            ////console.log("Modifying sentence");
            ////console.log("First 2 words:", words[0], words[1]);

            const firstWord = (words[0].trim().toLowerCase());

            if (questionWords.has(firstWord) || firstWord == "which" ||
                (words.length > 1 && questionWords.has(firstWord + " " + words[1].trim().toLowerCase()))
            ) 
            {
                msg = data += '?';
            } else {
                ////console.log("searching for colon/semicolon");
                ////console.log(words);
                let qFound = false;
                for (let i = 0; i < words.length; i += 1) {
                    if (words[i] == ':' || words[i] == ';') {
                        ////console.log("colon/semicolon found")
                        if (i + 1 == words.length) {
                            break;
                        }

                        const fw = words[i + 1];
                        let sw = '';
                        if ((i + 2) < words.length) {
                            sw = words[i + 2].trim().toLowerCase();
                        }
                        ////console.log("[" + fw + "][" + sw + "]");
                        if (questionWords.has(fw) || 
                            questionWords.has(fw + " " + sw.trim().toLowerCase())) {
                            msg = data + '?';
                            qFound = true;
                            break;
                        }

                    }
                }
                if (!qFound) {
                    msg = data += '.';
                }
            }
        } else {
            msg = data;
        }
        // const whichIdx = msg.indexOf("which");
        // if (whichIdx >= 0 && whichIdx != (msg.length - 1 - 5)) {
        //     if (whichIdx - 2 >= 0 && msg.charAt(whichIdx - 2) != ',') {
        //         msg = msg.substring(0, whichIdx - 1) + ", " + msg.substring(whichIdx);
        //     }
        // }
    }

    linguaWrite(id);
    linguaWrite(modCount); // modification count
    linguaWrite(mode);
    linguaWrite("speech");
    linguaWrite(msg);

    if (id != 0) {
        latestID = id;
        latestModCount = modCount;
    }

    if (logCommands) {
        console.log("handleSpeechInput command: {id:%d, modCount:%d mode:%s speech:%s}\n", id, modCount, mode, msg);
    }
}

const server = net.createServer((connection) => { 
    console.log('tcp client connected');
    let isActive = true;

    connection.setKeepAlive(true, 6000000);

    connections.push(connection);

    linguaWrite(0);
    linguaWrite(0); // modification count
    linguaWrite(0);
    linguaWrite("reset");
    linguaWrite("");
    latestID = -1;
    latestModCount = -1;
   
    connection.on('end', function() {
        console.log('tcp client disconnecting...');
        isActive = false;
    });
    connection.on('close', function() {
        console.log('tcp client closed');
        const index = connections.indexOf(connection);
        if (index > -1) {
          connections.splice(index, 1);
        }
        isActive = false;
    });
    connection.on('error', function() {
        console.log('client closed on error');
        isActive = false;
    });

    let prevData = "";
    let currData = "";

    let firstPing = true;
    connection.on("data", async (data) => {
        //console.log("data arrived");
        if (!isActive) {
            return;
        }

        const dataStr = data.toString();
        const cmds = dataStr.split("|");
        if (cmds[cmds.length - 1].length == 0) {
            cmds.pop();
        }
        // if (cmds.length > 1) {
        //     //console.log(cmds);
        // }
        for (let i = 0; i < cmds.length; i += 1) {
            const cmd = cmds[i].trim();

            const cmdName = cmd.substring(0, CMD_LENGTH);
            switch (cmdName) {
            case "PING": {
                // const payload = cmd.substring(CMD_LENGTH);
                // console.log("ping from", payload);
                //console.log("PING", cmd.substring(CMD_LENGTH));
                // if (firstPing == true) {
                //     readFile("./templates/default.json").then((data) => {
                //         console.log(data);

                //         for (let i = 0; i < connections.length; i += 1) {
                //             // load file
                //             connections[i].write("ldf:" + data + '\r\n'); // '\r\n' is the end-message marker
                //         }
                //     }).catch((err) => {
                //         console.error(err);
                //     });

                //     firstPing = false;
                // }
                break;
            }
            // synonym query
            case "SYNQ": {
                const payload = cmd.substring(CMD_LENGTH);
                const idSepIdx = payload.indexOf("&");

                const id = parseInt(payload.substring(0, idSepIdx));
                const msg = payload.substring(idSepIdx + 1).trim();
                console.log("SYNONYMS: ", id)
                linguaWrite(id);
                linguaWrite(0); // modification count
                linguaWrite(1);
                linguaWrite("synonym");
                linguaWrite(msg);

                //console.log("SYNONYM QUERY", msg);
                break;
            }
            // speech hypothesis
            case "SPHY": {
                handleSpeechInput(cmd, 0)
                break;
            }
            // speech result
            case "SPRE": {
                handleSpeechInput(cmd, 1)
                latestModCount = -1;
                break;
            }
            // speech reset
            case "SPRT": {
                // unused
                break;
            }
            case "LOAD": {
                
                readFile(DEFAULT_TEMPLATE_PATH).then((data) => {
                    console.log(data);

                    let regex = /\,(?=\s*?[\}\]])/g;
                    data = data.replace(regex, ''); // remove all trailing commas

                    if (!isJSONString(data)) {
                        console.error("invalid file");
                        return;
                    }

                    for (let i = 0; i < connections.length; i += 1) {
                        // load file
                        connections[i].write("ldf:" + DEFAULT_TEMPLATE_PATH + "|" + data + '\r\n'); // '\r\n' is the end-message marker
                    }
                }).catch((err) => {
                    console.error(err);
                });

                break;
            }
            case "MESG": {
                const payload = cmd.substring(CMD_LENGTH);

                const idSepIdx = payload.indexOf("&");

                const type = payload.substring(0, idSepIdx);
                //const data = payload.substring(idSepIdx + 1).trim();
                
                if (type == "reset") {
                    console.log("client request to reset lingua");
                    linguaWrite(0);
                    linguaWrite(0); // modification count
                    linguaWrite(0);
                    linguaWrite(type);
                    linguaWrite("");
                } else if (type == "dyld") {
                    console.log("dynamic load requested");
                    //let dyldArgs = JSON.parse(payload.substring(idSepIdx + 1));
                    //console.log(dyldArgs);
                    dynamicLoad(connections);
                }

                break;
            }
            default: {
                console.error("ERROR: unknown case", cmdName);
                break;
            }
            }
        }
        
        //connection.write("Hello World!\r\n");
    });
   
    if (!isActive) {
        return;
    }
    connection.write('PING\r\n');
   // connection.pipe(connection);
});

const extSet = new Set();

const cmdServer = net.createServer((connection) => { 
    console.log('tcp client connected (cmd port)');
    let isActive = true;

    connection.setKeepAlive(true, 6000000);

    cmdConnections.push(connection);
   
    connection.on('end', function() {
        console.log('tcp client disconnecting... (cmd)');
        isActive = false;
    });
    connection.on('close', function() {
        console.log('tcp client closed (cmd)');
        const index = cmdConnections.indexOf(connection);
        if (index > -1) {
          cmdConnections.splice(index, 1);
        }
        isActive = false;
    });
    connection.on('error', function() {
        console.log('client closed on error (cmd)');
        isActive = false;
    });

    connection.on("data", async (data) => {
        if (!isActive) {
            return;
        }

        const dataStr = data.toString();
        const cmds = dataStr.split("|");
        if (cmds[cmds.length - 1].length == 0) {
            cmds.pop();
        }
        if (cmds.length > 1) {
            //console.log(cmds);
        }
        for (let i = 0; i < cmds.length; i += 1) {
            const cmd = cmds[i].trim();
            const cmdName = cmd.substring(0, CMD_LENGTH);
            
            switch (cmdName) {
            case "PING": {
                // const payload = cmd.substring(CMD_LENGTH);
                // console.log("ping from", payload);
                break;
            }
            case "MESG": {
                const payload = cmd.substring(CMD_LENGTH);

                const idSepIdx = payload.indexOf("&");

                const type = payload.substring(0, idSepIdx);
                //const data = payload.substring(idSepIdx + 1).trim();
                
                if (type == "reset") {
                    console.log("(command) client request to reset lingua");
                    linguaWrite(0);
                    linguaWrite(0); // modification count
                    linguaWrite(0);
                    linguaWrite(type);
                    linguaWrite("");
                } else if (type == "dyld") {
                    console.log("dynamic load requested");
                    //let dyldArgs = JSON.parse(payload.substring(idSepIdx + 1));
                    //console.log(dyldArgs);
                    dynamicLoad(connections);
                }

                break;
            }
            case "EXFR": {
                // console.log("received from external");
                // console.log(connection.remoteAddress,
                //     connection.remotePort,
                //     connection.localAddress,
                //     connection.localPort);
                if (!extSet.has(connection)) {
                    connection.write("IDST{type:id_set, val:1}\r\n");
                }
                extSet.add(connection);

                const payload = cmd.substring(CMD_LENGTH);
                
                for (let conn_i = 0; conn_i < cmdConnections.length; conn_i += 1) {
                    const conn = cmdConnections[conn_i];
                    if (extSet.has(conn)) {
                        continue;
                    }

                    conn.write(cmd + "\r\n");
                }
                break;
            }
            case "EXTO": {

                const payloadOut = cmd.substring(CMD_LENGTH) + "\r\n";

                for (let extConnection of extSet) {
                    extConnection.write(payloadOut);
                }
                break;
            }
            default: { 
                console.error("ERROR: unknown case", cmdName);
                break; 
            }
            }
        }
    })
});

const SERVER_PORT = 8080;
server.listen(SERVER_PORT, "0.0.0.0", function() {
    console.log('tcp server is listening on port', SERVER_PORT);
});
cmdServer.listen((SERVER_PORT + 1), "0.0.0.0", function() {
    console.log('tcp server (cmd) is listening on port', (SERVER_PORT + 1));
});

setInterval(() => {
    for (let i = 0; i < connections.length; i += 1) {
        connections[i].write('PING\r\n');
    }
    for (let i = 0; i < cmdConnections.length; i += 1) {
        cmdConnections[i].write('PING\r\n');
    }
}, 30000);

function onExit() {
    console.log("Exiting");
    if (pyProgMain != null) {
        pyProgMain.stdin.end();
        pyProgMain.kill('SIGINT');
        pyProgMain.kill();
    }

    if (pyProgAux != null) {
        pyProgAux.stdin.write("exit\n");
        pyProgAux.stdin.end();
        pyProgAux.kill('SIGINT');
        pyProgAux.kill();
    }


    rl.close();
    process.exit();
}

process.on('SIGINT', onExit);

rl.on('SIGINT', () => {
    rl.close();
    onExit();
});
rl.on('SIGQUIT', () => { 
   rl.close();
    onExit();
});
rl.on('SIGTERM', () => {
    rl.close();
    onExit();
});


initLingua();

}


main();
