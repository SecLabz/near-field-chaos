let portOpen = false;
let holdPort = null;
let port;
let reader;

let responseCallback = null;
let errorCallback = null;

let textArea;
let fileInput;

const TOAST_TYPE_ERROR = 0;
const TOAST_TYPE_SUCCESS = 1;

function showToast(message, type) {
  Toastify({
    text: message,
    gravity: "bottom",
    position: "right",
    close: true,
    style: {
      background:
        type === TOAST_TYPE_SUCCESS
          ? "linear-gradient(to right, rgb(52 211 153), rgb(16 185 129))"
          : "linear-gradient(to right, rgb(190 18 60), #ff5f6d)",
    },
  }).showToast();
}

window.context = {};

window.onload = function () {
  if ("serial" in navigator) {
    document
      .getElementById("openclose_port")
      .addEventListener("click", openClose);
    document.getElementById("read_tag").addEventListener("click", readTag);
    document.getElementById("write_tag").addEventListener("click", writeTag);
    document.getElementById("clear").addEventListener("click", clear);
    document.getElementById("save_tag").addEventListener("click", saveTag);
    textArea = document.getElementById("dump_textarea");
    fileInput = document.getElementById("fileInput");
    fileInput.addEventListener("change", onFileChange);
  } else {
    alert("The Web Serial API is not supported by your browser");
  }
  init_blank_tag_content();
  modalLoading.init(true);
  document.title = "🔴 ST25TB Reader/Writer";
};

navigator.serial.addEventListener("connect", async (event) => {
  holdPort = event.target;

  openClose();
});

function changeConnectedState(connected) {
  if (connected) {
    document.getElementById("openclose_port").innerText = "Close";
    document.getElementById("openclose_port").style =
      "background-color:rgb(190 18 60);";
    document.getElementById("read_tag").disabled = false;
    document.getElementById("write_tag").disabled = false;
    document.getElementById("clear").disabled = false;
    document.title = "🟢 ST25TB Reader/Writer";
  } else {
    document.getElementById("openclose_port").innerText = "Open";
    document.getElementById("openclose_port").style = "";
    document.getElementById("read_tag").disabled = true;
    document.getElementById("write_tag").disabled = true;
    document.getElementById("clear").disabled = true;
    document.getElementById("save_tag").disabled = true;
    document.title = "🔴 ST25TB Reader/Writer";
  }
}

async function openClose() {
  if (portOpen) {
    reader.cancel();
    console.log("attempt to close");
  } else {
    return new Promise((resolve) => {
      (async () => {
        if (holdPort == null) {
          port = await navigator.serial.requestPort();
          holdPort = port;
        } else {
          port = holdPort;
          holdPort = null;
        }
        await port.open({ baudRate: 115200 });

        const textDecoder = new TextDecoderStream();
        reader = textDecoder.readable.getReader();
        const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);

        portOpen = true;
        changeConnectedState(true);

        let result = "";
        let log = "";
        while (true) {
          const { value, done } = await reader.read();
          if (done) {
            reader.releaseLock();
            break;
          }

          result += value;
          log += value;

          if (log.indexOf("\n") !== -1) {
            console.log(log.substring(0, log.indexOf("\r\n")));
            log = log.substring(log.indexOf("\n") + 1, log.length);
          }

          let match = result.match(/RESPONSE:([\s\S]+?)(?:\r?\n)*nfc> /);
          if (match && responseCallback) {
            responseCallback(match[1]);
            result = "";
            continue;
          }

          match = result.match(/ERROR:([\s\S]+?)(?:\r?\n)*nfc> /);
          if (match && errorCallback) {
            errorCallback(match[1]);
            result = "";
          }
        }

        await readableStreamClosed.catch(() => {});
        await port.close();

        portOpen = false;
        changeConnectedState(false);
        resolve();
      })().catch((e) => {
        console.log(e);
        portOpen = false;
        changeConnectedState(false);
      });
    });
  }
}

function getTagContent() {
  let tag_content = document
    .getElementById("dump_textarea")
    .value.replace(/[^0-9a-fA-F]/g, "");
  if (tag_content.length != 136) {
    return null;
  }
  return tag_content;
}

function init_blank_tag_content() {
  let i;
  document.getElementById("uid").innerHTML = "UID: XXXXXXXXXXXXXXXX";
  document.getElementById("dump_textarea").value = "";
  for (i = 0; i <= 16; i++) {
    document.getElementById("dump_textarea").value += "xxxxxxxx\n";
  }
  document.getElementById("name").disabled = true;
  document.getElementById("save_tag").disabled = true;
  document.getElementById("name").value = "";
}

async function clear() {
  init_blank_tag_content();
}

function getFormattedDate() {
  const today = new Date();
  const dd = String(today.getDate()).padStart(2, "0");
  const mm = String(today.getMonth() + 1).padStart(2, "0");
  const yyyy = today.getFullYear();
  return mm + "-" + dd + "-" + yyyy;
}

function updateSaveName() {
  document.getElementById("name").disabled = false;
  document.getElementById("save_tag").disabled = false;

  const date = getFormattedDate();
  document.getElementById(
    "name"
  ).value = `dump_${window.context.uid}_${date}.nfc`;
}

async function loadTagFromFile(str) {
  const uidRegex = /^uid\s*:\s*([a-fA-F0-9]{16})/i;
  const tagContentRegex = /^[a-fA-F0-9]{8}/;
  let values = [];
  let uid = null;
  for (let line of str.split("\n")) {
    if (uidRegex.test(line)) {
      const match = line.match(uidRegex);
      uid = match[1];
    } else if (tagContentRegex.test(line)) {
      const match = line.match(tagContentRegex);
      values.push(match[0]);
    }
  }

  if (uid != null && values.length == 17) {
    window.context.uid = uid;
    document.getElementById("uid").innerHTML = "UID: " + uid;
    textArea.value = "";
    for (let v of values) {
      textArea.value += v + "\n";
    }
  } else {
    alert("error");
  }

  updateSaveName();
}

function onFileChange(event) {
  const file = event.target.files[0];

  if (file) {
    const reader = new FileReader();

    reader.onload = function (e) {
      loadTagFromFile(e.target.result);
    };

    reader.readAsText(file);
  }
}

async function sendCommand(command) {
  const textEncoder = new TextEncoderStream();
  const writableStreamClosed = textEncoder.readable.pipeTo(port.writable);
  const writer = textEncoder.writable.getWriter();

  const promise = new Promise((resolve, reject) => {
    responseCallback = resolve;
    errorCallback = reject;
  });

  await writer.write(command + String.fromCharCode(13));

  writer.close();
  await writableStreamClosed;
  return promise;
}

async function readTag() {
  let tag;
  try {
    tag = await sendCommand("read_tag_raw");
  } catch (e) {
    showToast(e, TOAST_TYPE_ERROR);
    return;
  }
  document.getElementById("dump_textarea").value = "";

  if (tag && tag.length == 152) {
    for (i = 0; i < 17; i++) {
      document.getElementById("dump_textarea").value +=
        tag.substring(i * 8, i * 8 + 8) + "\n";
    }
  }
  let uid = tag.substring(tag.length - 16, tag.length);
  window.context.uid = uid;
  document.getElementById("uid").innerHTML = "UID: " + uid;
  updateSaveName();
  showToast("Tag read successfully", TOAST_TYPE_SUCCESS);
}

async function writeTag() {
  const tag_content = getTagContent();

  let response;
  modalLoading.show();
  try {
    if (!window.context.uid || window.context.uid.length !== 16) {
      showToast("Missing uid", TOAST_TYPE_ERROR);
      return;
    }
    response = await sendCommand(`write_tag_raw ${tag_content}${window.context.uid}`);
  } catch (e) {
    modalLoading.hide();
    errorToast(e);
    showToast(e, TOAST_TYPE_ERROR);
    return;
  }

  modalLoading.hide();
  if (response) {
    showToast(response, TOAST_TYPE_SUCCESS);
  }
}

async function saveTag() {
  const tag = getTagContent();
  if (tag === null && window.context.uid.length !== 16) {
    showToast("Can't save. Maybe wrong tag content ?", TOAST_TYPE_ERROR);
    return;
  }

  let fileContent = `UID: ${window.context.uid}\n`;
  for (i = 0; i < 17; i++) {
    fileContent += tag.substring(i * 8, i * 8 + 8) + "\n";
  }

  const fileName = document.getElementById("name").value.trim();

  const blob = new Blob([fileContent], {
    type: "text/plain",
  });

  const link = document.createElement("a");
  link.href = URL.createObjectURL(blob);
  link.download = fileName;

  link.click();
  URL.revokeObjectURL(link.href);
}
