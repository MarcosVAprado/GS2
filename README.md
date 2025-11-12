# GS 2025 - Edge Computing & Computer Systems
## Projeto: Sistema Inteligente de Gestão de Bem-Estar para Home Office

### Integrantes
*   Artur Rodrigues Trindade Paes - RM: 564309
*   Gabriel Silva Novais - RM: 566370
*   Marcos Vinicius Aquino Prado - RM: 562775

---

### 1. Descrição do Problema
A popularização do home office trouxe flexibilidade, mas também novos desafios para a saúde e o bem-estar dos profissionais. Longas jornadas sem pausas, má postura, e ambientes de trabalho inadequados (iluminação e temperatura) podem levar ao esgotamento (burnout), dores crônicas e queda de produtividade. Este projeto aborda a necessidade de criar um ambiente de trabalho remoto mais saudável e consciente, utilizando tecnologia para monitorar e fornecer feedback em tempo real ao usuário.

### 2. Descrição da Solução
O **Sistema Inteligente de Gestão de Bem-Estar** é uma solução IoT baseada em ESP32 que monitora ativamente o ambiente de trabalho e os hábitos do usuário para promover um dia de trabalho mais equilibrado.

**Funcionalidades Principais:**
*   **Monitoramento Ambiental:** Sensores de temperatura (`DHT22`) e luminosidade (`LDR`) garantem que o ambiente esteja em condições ideais.
*   **Controle de Postura:** Um sensor ultrassônico (`HC-SR04`) detecta a distância do usuário, alertando sobre posturas inadequadas mantidas por muito tempo.
*   **Gestão de Pausas Inteligente:** O sistema implementa a técnica Pomodoro, alternando ciclos de trabalho e descanso, com indicadores visuais (LEDs) e sonoros (buzzer) para guiar o usuário.
*   **Comunicação IoT:** Todos os dados coletados são publicados via protocolo **MQTT**, permitindo a integração com dashboards de monitoramento e outras aplicações externas para análise do histórico de bem-estar.

### 3. Instruções de Uso e Dependências
O projeto foi desenvolvido e simulado na plataforma Wokwi.

**Link para a Simulação no Wokwi:**
*   [**Clique aqui para acessar o projeto no Wokwi**](https://wokwi.com/projects/447434894669372417)
**Dependências (Bibliotecas Arduino):**
*   `WiFi.h` (padrão do ESP32)
*   `PubSubClient.h` (para comunicação MQTT)
*   `DHT.h` (para o sensor DHT22)

Para executar, basta abrir o link da simulação e clicar no botão "Start Simulation". A saída serial mostrará o status da conexão Wi-Fi, MQTT e as leituras dos sensores.

### 4. Imagens do Circuito
*(<img width="908" height="398" alt="image" src="https://github.com/user-attachments/assets/47a06919-1499-41a5-92d2-adbef9db4d81" />
)*

### 5. Tópicos MQTT Utilizados
O sistema utiliza um broker MQTT público (`broker.hivemq.com`) para publicar dados nos seguintes tópicos:

*   `fiap/gs/bem_estar/temperatura`: Publica o valor da temperatura em Celsius.
*   `fiap/gs/bem_estar/umidade`: Publica o valor da umidade relativa (%).
*   `fiap/gs/bem_estar/luminosidade`: Publica o valor bruto lido pelo LDR.
*   `fiap/gs/bem_estar/distancia`: Publica a distância do usuário em centímetros.
*   `fiap/gs/bem_estar/status`: Publica mensagens de status do sistema (ex: "Pausa iniciada", "Alerta de Postura").
