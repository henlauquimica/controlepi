import { initializeApp } from "https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js";
import { getDatabase, ref, onChildAdded, onValue, query, limitToLast } from "https://www.gstatic.com/firebasejs/10.7.1/firebase-database.js";

// TODO: Replace with your Firebase Project Config
const firebaseConfig = {
    apiKey: "AIzaSyB830NJZsrcvGvKo-_2i8QgAfptRDKRdLM",
    authDomain: "gen-lang-client-0335555331.firebaseapp.com",
    databaseURL: "https://gen-lang-client-0335555331-default-rtdb.firebaseio.com",
    projectId: "gen-lang-client-0335555331",
    storageBucket: "gen-lang-client-0335555331.firebasestorage.app",
    messagingSenderId: "677638342514",
    appId: "1:677638342514:web:64e4c871ee10855190ffeb",
    measurementId: "G-NRY6NYDL9V"
};

// Initialize Firebase
let app;
let db;

function initFirebase() {
    try {
        app = initializeApp(firebaseConfig);
        db = getDatabase(app);
        updateStatus(true);
        listenForData();
    } catch (error) {
        console.error("Firebase Init Error:", error);
        updateStatus(false);
    }
}

function updateStatus(connected) {
    const el = document.getElementById('connectionStatus');
    const text = el.querySelector('.text');
    if (connected && firebaseConfig.apiKey !== "YOUR_API_KEY") {
        el.classList.add('connected');
        text.innerText = "Conectado";
    } else {
        el.classList.remove('connected');
        text.innerText = "Modo Demo / Configurar";
        if (firebaseConfig.apiKey === "YOUR_API_KEY") {
            simulateData(); // Run simulation if no config
        }
    }
}

function listenForData() {
    const logsRef = query(ref(db, 'logs'), limitToLast(20));
    
    // Listen for new logs
    onChildAdded(logsRef, (snapshot) => {
        const data = snapshot.val();
        addLogRow(data);
        updateStats(data);
    });
}

const EMPLOYEES = {
    "A1 B2 C3 D4": "Rafael (Gerente)",
    "E5 F6 G7": "Funcionário Exemplo 1",
    "99 88 77": "Funcionário Exemplo 2",
    "TESTE_SISTEMA": "Teste Automático"
};

// UI Functions
function addLogRow(data) {
    const tbody = document.getElementById('logTableBody');
    const placeholder = tbody.querySelector('.placeholder-row');
    if (placeholder) placeholder.remove();

    const tr = document.createElement('tr');
    tr.className = 'new-row';
    
    // Determine status style
    const isSuccess = data.status_uso === "Sucesso";
    const statusClass = isSuccess ? 'success' : 'error';
    
    // Map ID to Name
    const name = EMPLOYEES[data.id_funcionario] || data.id_funcionario;
    const displayName = EMPLOYEES[data.id_funcionario] ? `<b>${name}</b> <small>(${data.id_funcionario})</small>` : `<code>${data.id_funcionario}</code>`;

    tr.innerHTML = `
        <td>${data.horario || new Date().toLocaleTimeString()}</td>
        <td>${displayName}</td>
        <td><span class="status-badge ${statusClass}">${data.status_uso}</span></td>
        <td>${isSuccess ? 'Dispensação Liberada' : 'Tempo esgotado (Sem mão)'}</td>
    `;
    
    tbody.insertBefore(tr, tbody.firstChild);

    // Keep table size manageable
    if (tbody.children.length > 20) {
        tbody.lastChild.remove();
    }
}

let dailyCount = 0;
let alertCount = 0;

function updateStats(data) {
    // Update counters
    if (data.status_uso === "Sucesso") {
        dailyCount++;
        document.getElementById('todayCount').textContent = dailyCount;
    } else {
        alertCount++;
        document.getElementById('alertCount').textContent = alertCount;
    }
    
    // Update last access
    document.getElementById('lastAccess').textContent = data.horario ? data.horario.split(' ')[1] : new Date().toLocaleTimeString();
}

// Simulation for Demo purposes (Preview without Hardware)
function simulateData() {
    // SECURITY: Ensure we NEVER run simulation if the API Key is real.
    if (firebaseConfig.apiKey !== "YOUR_API_KEY") {
        console.warn("Simulation blocked: Real API Key detected.");
        return; 
    }

    console.log("Starting Simulation Mode...");
    const mockIds = ["A1 B2 C3", "E5 F6 G7", "99 88 77"];
    
    setInterval(() => {
        const isSuccess = Math.random() > 0.2;
        const mockData = {
            id_funcionario: mockIds[Math.floor(Math.random() * mockIds.length)],
            status_uso: isSuccess ? "Sucesso" : "Timeout",
            horario: new Date().toLocaleString('pt-BR')
        };
        addLogRow(mockData);
        updateStats(mockData);
    }, 5000);
}

// Start
document.addEventListener('DOMContentLoaded', initFirebase);
