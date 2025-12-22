# 🧴 ControlEPI - Smart Dispense System

Bem-vindo ao repositório oficial do **ControlEPI**, um sistema automatizado para controle de dispensação de insumos (como EPIs ou higienizadores) baseado em autenticação RFID e detecção de presença. Este projeto foi desenvolvido para a **Henlau Química** utilizando o microcontrolador ESP32.

---

## 📋 Sobre o Projeto

O **ControlEPI** visa garantir que apenas funcionários autorizados tenham acesso ao dispenser e que o procedimento de retirada seja realizado corretamente. O sistema integra autenticação segura via cartões RFID com sensores de infravermelho para detectar a mão do usuário, garantindo uma operação "sem toque" (touchless) após a liberação.

## 🚀 Funcionalidades

*   **Autenticação Via RFID**: Controle de acesso restrito a UIDs (cartões) cadastrados.
*   **Detecção de Presença (Anti-desperdício)**: O dispenser só é acionado se o usuário posicionar a mão no local correto após a autenticação.
*   **Feedback Sonoro Interativo**:
    *   *Bip Curto*: Autenticação bem-sucedida.
    *   *Bips Intermitentes*: Aguardando posicionamento da mão.
    *   *Bip Longo*: Liberação do dispenser.
    *   *Alarme (3s)*: Erro de procedimento (Timeout).
*   **Acionamento via Solenoide**: Controle preciso de trava elétrica/válvula.

---

## 🛠️ Hardware Necessário

O projeto é baseado na plataforma **ESP32** e utiliza os seguintes periféricos:

*   **Microcontrolador**: ESP32 Development Board (DOIT DEVKIT V1)
*   **Leitor RFID**: Módulo RC522 (SPI)
*   **Sensor de Presença**: Sensor Infravermelho (Obstacle/Line Sensor digital)
*   **Atuador**: Módulo Relé ou Driver MOSFET para Solenoide
*   **Feedback**: Buzzer Ativo

---

## 🔌 Pinagem (Pinout)

A configuração padrão dos pinos definida no firmware (`src/main.cpp`) é:

| Periférico | Pino ESP32 | Função |
| :--- | :--- | :--- |
| **RFID SDA (SS)** | GPIO 21 | Chip Select do MFRC522 |
| **RFID SCK** | GPIO 18 | SPI Clock |
| **RFID MOSI** | GPIO 23 | SPI MOSI |
| **RFID MISO** | GPIO 19 | SPI MISO |
| **RFID RST** | GPIO 22 | Reset do MFRC522 |
| **Solenoide (Relé)**| GPIO 4 | Controle de Ativação (Ativo em LOW*) |
| **Buzzer** | GPIO 12 | Saída para Buzzer Ativo |
| **Sensor IR** | GPIO 13 | Entrada do Sensor (Ativo em LOW) |

*\*Nota: O código assume que o Relé é ativado com nível lógico LOW (lógica invertida comum em módulos relé).*

---

## 🧠 Lógica de Funcionamento (Técnico)

O firmware opera em um loop contínuo de verificação com a seguinte máquina de estados:

1.  **Standby**: O sistema aguarda a aproximação de uma tag RFID válida.
2.  **Autenticação**:
    *   Ao ler uma tag, o UID é comparado com a lista `CARTOES_LIBERADOS`.
    *   **Inválido**: O sistema ignora e loga no Serial.
    *   **Válido**: Emite aviso sonoro e inicia a janela de tempo (`TEMPO_ESPERA_MAO`).
3.  **Janela de Espera (10s)**:
    *   O sistema aguarda que o **Sensor IR** detecte a mão (sinal `LOW`).
    *   Durante a espera, o buzzer emite bips curtos de orientação a cada 500ms.
4.  **Liberação ou Timeout**:
    *   **Mão Detectada**: Chama `liberarDispenser()`. O relé é acionado por 5 segundos, permitindo a retirada do produto.
    *   **Timeout (10s sem mão)**: O sistema entra em modo de falha, emitindo um alarme contínuo de 3 segundos para alertar sobre o procedimento incorreto.

---

## 💻 Como Compilar e Enviar

Este projeto utiliza o **PlatformIO** para gerenciamento de dependências e compilação.

### Pré-requisitos
*   VS Code com a extensão PlatformIO instalada.

### Passos
1.  Clone este repositório:
    ```bash
    git clone https://github.com/henlauquimica/controlepi.git
    ```
2.  Abra a pasta do projeto no VS Code.
3.  O PlatformIO baixará automaticamente a biblioteca `miguelbalboa/MFRC522` corrigida.
4.  Conecte o ESP32 via USB.
5.  Clique no ícone **PlatformIO** (formiga) e selecione:
    *   **Build**: Para compilar.
    *   **Upload**: Para gravar no ESP32.

6.  Visualize o Dashboard:
    *   **Localmente**: Abra o arquivo `dashboard/index.html` no seu navegador.
    *   **Online**: Após configurar o Firebase, acesse a URL gerada pelo comando `firebase deploy`.

---

## 📄 Licença
Desenvolvido para **Henlau Química**. Todos os direitos reservados.

