import { createClient } from 'https://cdn.jsdelivr.net/npm/@supabase/supabase-js@2/+esm';

// Configuração do Supabase (Mesmas credenciais do ESP32)
const SUPABASE_URL = "https://inwskxdquwfhhryxpghh.supabase.co";
const SUPABASE_ANON_KEY = "sb_publishable_gQTZhkfL1mcJtrVN_uuIrw_N6UJp82F";

// ID do funcionário mapeado para nomes (Exemplo)
const EMPLOYEES = {
    "A1 B2 C3 D4": "Rafael (Gerente)",
    "E5 F6 G7": "Funcionário Exemplo 1",
    "99 88 77": "Funcionário Exemplo 2",
    "TESTE_SISTEMA": "Teste Automático"
};

let supabase;

function initSupabase() {
    try {
        supabase = createClient(SUPABASE_URL, SUPABASE_ANON_KEY);
        updateStatus(true);
        listenForData();
        fetchInitialData();
    } catch (error) {
        console.error("Supabase Init Error:", error);
        updateStatus(false);
    }
}

function updateStatus(connected) {
    const el = document.getElementById('connectionStatus');
    const text = el.querySelector('.text');
    if (connected) {
        el.classList.add('connected');
        text.innerText = "Conectado (Supabase)";
    } else {
        el.classList.remove('connected');
        text.innerText = "Desconectado";
    }
}

async function fetchInitialData() {
    // Buscar os últimos 20 registros
    const { data, error } = await supabase
        .from('logs')
        .select('*')
        .order('created_at', { ascending: false })
        .limit(20);

    if (error) {
        console.error("Erro ao buscar dados iniciais:", error);
        return;
    }

    // Inverter para mostrar a ordem correta se inserirmos no topo?
    // A função addLogRow insere no topo (insertBefore), então se pegar os ultimos 20 (mais novos),
    // o [0] é o mais novo. Se iterarmos do ultimo pro primeiro, o mais novo fica no topo.
    // data está DESC (mais novo primeiro).
    // Se iterarmos normal (mais novo -> mais velho), o mais novo será o último inserido no topo?
    // Não, addLogRow(row) -> insertBefore(firstChild).
    // Então o último que chamarmos addLogRow será o primeiro da tabela.
    // Queremos o mais novo no topo.
    // Então devemos processar do mais VELHO para o mais NOVO.
    
    // Reverse array to process oldest first, so newest ends up at top
    const dataReverse = [...data].reverse();
    dataReverse.forEach(log => {
        addLogRow(log);
        updateStats(log, true); // true = isHistory
    });
}

function listenForData() {
    // Escutar novos inserts na tabela 'logs'
    supabase
        .channel('public:logs')
        .on('postgres_changes', { event: 'INSERT', schema: 'public', table: 'logs' }, payload => {
            console.log('Novo log recebido:', payload.new);
            addLogRow(payload.new);
            updateStats(payload.new, false);
        })
        .subscribe((status) => {
            if (status === 'SUBSCRIBED') {
                console.log('Ouvindo novos dados em tempo real...');
            }
        });
}

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
    // Tenta formatar bonito se for um ID conhecido, senão mostra o ID
    const displayName = EMPLOYEES[data.id_funcionario] ? `<b>${name}</b> <small>(${data.id_funcionario})</small>` : `<code>${data.id_funcionario}</code>`;

    // Formatar horário
    // Supabase envia 'created_at' em UTC ISO string ex: "2023-10-27T10:00:00.000Z"
    let timeStr = "---";
    if (data.created_at) {
        timeStr = new Date(data.created_at).toLocaleTimeString('pt-BR');
    } else if (data.horario) { // Fallback se tiver o campo antigo
        timeStr = data.horario;
    } else {
        timeStr = new Date().toLocaleTimeString('pt-BR');
    }

    tr.innerHTML = `
        <td>${timeStr}</td>
        <td>${displayName}</td>
        <td><span class="status-badge ${statusClass}">${data.status_uso}</span></td>
        <td>${isSuccess ? 'Dispensação Liberada' : 'Tempo esgotado (Sem mão)'}</td>
    `;
    
    // Insere sempre no topo
    tbody.insertBefore(tr, tbody.firstChild);

    // Keep table size manageable
    if (tbody.children.length > 20) {
        tbody.lastChild.remove();
    }
}

let dailyCount = 0;
let alertCount = 0;

function updateStats(data, isHistory) {
    // Se for dado histórico, talvez não queiramos somar no contador de "Hoje" se for de outro dia?
    // Para simplificar, vamos assumir que queremos contar tudo que aparece ou melhorar a logica depois.
    // Vamos contar apenas se for "Hoje"
    
    const now = new Date();
    const logDate = data.created_at ? new Date(data.created_at) : new Date();
    
    // Verifica se é o mesmo dia
    const isToday = now.toDateString() === logDate.toDateString();
    
    if (isToday) {
        if (data.status_uso === "Sucesso") {
            dailyCount++;
            document.getElementById('todayCount').textContent = dailyCount;
        } else {
            alertCount++;
            document.getElementById('alertCount').textContent = alertCount;
        }
    }
    
    // Update last access (apenas se for evento novo ou o mais recente do historico)
    // Essa lógica atualiza sempre. No carregamento inicial (histórico), o ultimo processado é o mais novo, então fica correto.
    document.getElementById('lastAccess').textContent = logDate.toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit'});
}

// Start
document.addEventListener('DOMContentLoaded', () => {
    initSupabase();

    // Mobile Menu Logic
    const menuBtn = document.getElementById('menuBtn');
    const closeBtn = document.getElementById('closeBtn');
    const sidebar = document.getElementById('sidebar');
    const overlay = document.getElementById('menuOverlay');

    function toggleMenu() {
        const isActive = sidebar.classList.contains('active');
        if (isActive) {
            sidebar.classList.remove('active');
            overlay.classList.remove('active');
        } else {
            sidebar.classList.add('active');
            overlay.classList.add('active');
        }
    }

    if (menuBtn) menuBtn.addEventListener('click', toggleMenu);
    if (closeBtn) closeBtn.addEventListener('click', toggleMenu);
    if (overlay) overlay.addEventListener('click', toggleMenu);
});
