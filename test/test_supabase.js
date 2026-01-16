const https = require('https');

const SUPABASE_URL = "https://inwskxdquwfhhryxpghh.supabase.co";
const SUPABASE_ANON_KEY = "sb_publishable_gQTZhkfL1mcJtrVN_uuIrw_N6UJp82F";

// Determine endpoint
const endpoint = `${SUPABASE_URL}/rest/v1/logs`;

const mockData = {
    id_funcionario: "TESTE_NODE_" + Math.floor(Math.random() * 1000),
    status_uso: "Sucesso",
    // Supabase usually handles created_at automatically if defined, but we can send it or let db handle it.
    // The ESP32 code sends: id_funcionario, status_uso.
};

const dataString = JSON.stringify(mockData);

const urlObj = new URL(endpoint);

const options = {
    hostname: urlObj.hostname,
    path: urlObj.pathname,
    method: 'POST',
    headers: {
        'Content-Type': 'application/json',
        'apikey': SUPABASE_ANON_KEY,
        'Authorization': `Bearer ${SUPABASE_ANON_KEY}`,
        'Prefer': 'return-minimal',
        'Content-Length': dataString.length
    }
};

const req = https.request(options, (res) => {
    console.log(`Status Code: ${res.statusCode}`);
    
    res.on('data', (d) => {
        process.stdout.write(d);
    });

    if (res.statusCode === 201) {
        console.log("\nSUCCESS: Mock data sent to Supabase!");
    } else {
        console.log("\nFAILURE: Check configuration.");
    }
});

req.on('error', (e) => {
    console.error(e);
});

req.write(dataString);
req.end();
