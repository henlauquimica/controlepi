const https = require('https');

const SUPABASE_URL = "https://inwskxdquwfhhryxpghh.supabase.co";
const SUPABASE_ANON_KEY = "sb_publishable_gQTZhkfL1mcJtrVN_uuIrw_N6UJp82F";

// Determine endpoint
const endpoint = `${SUPABASE_URL}/rest/v1/logs?select=*&limit=5&order=created_at.desc`;

const urlObj = new URL(endpoint);

const options = {
    hostname: urlObj.hostname,
    path: urlObj.pathname + urlObj.search,
    method: 'GET',
    headers: {
        'apikey': SUPABASE_ANON_KEY,
        'Authorization': `Bearer ${SUPABASE_ANON_KEY}`,
        'Prefer': 'return-minimal'
    }
};

const req = https.request(options, (res) => {
    console.log(`Status Code: ${res.statusCode}`);
    
    let data = '';
    res.on('data', (d) => {
        data += d;
    });
    
    res.on('end', () => {
        console.log("Response Body:");
        console.log(data);
    });
});

req.on('error', (e) => {
    console.error(e);
});

req.end();
