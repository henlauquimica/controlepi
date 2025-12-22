const https = require('https');

const data = JSON.stringify({
  id_funcionario: "99 88 77",
  status_uso: "Timeout",
  horario: new Date().toLocaleString()
});

const url = 'https://gen-lang-client-0335555331-default-rtdb.firebaseio.com/logs.json';

const req = https.request(url, {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': data.length
  }
}, (res) => {
  console.log(`StatusCode: ${res.statusCode}`);
  
  res.on('data', (d) => {
    process.stdout.write(d);
  });
});

req.on('error', (error) => {
  console.error(error);
  process.exit(1);
});

req.write(data);
req.end();
