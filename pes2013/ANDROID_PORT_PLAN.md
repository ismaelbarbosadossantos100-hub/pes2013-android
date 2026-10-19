# PES 2013 Android — plano técnico

## Alvo principal

- ABI: `arm64-v8a`.

- SoC-alvo: MediaTek Helio G99 com GPU Mali-G57.

- Backend padrão: Vulkan do sistema Android.

- Backend experimental: PanVK/Panfrost somente quando o dispositivo e o ambiente oferecerem suporte compatível.

- Não usar drivers Adreno em Mali-G57; `Adrenotools` é uma camada específica para GPUs Adreno e não deve ser carregada neste alvo.

## Resolução personalizada

A configuração deve separar resolução interna do jogo e resolução da superfície Android:

- `internal_width` / `internal_height`: resolução de renderização d guest.

- `surface_width` / `surface_height`: tamanho físico da `ANativeWindow`.

- Presets: nativo, 720p, 900p, 1080p e personalizado.

- Escala dinâmica opcional: 50–100%, com limite de FPS e ajuste de frame pacing.

- Letterbox/pillarbox para preservar a proporção 16:9.

- Salvar em `SharedPreferences`/TOML no diretório privado do app.

- Recriar swapchain apenas no thread de renderização quando a superfície mudar.

## Controles na tela

Overlay inspirado no controle Xbox 360:

- Analógico esquerdo e direito com zonas mortas ajustáveis.

- D-pad em quatro direções.

- A, B, X, Y.

- LB, RB, LT, RT.

- Back, Start e Guide opcional.

- Opacidade, escala, posição e vibração configuráveis.

- Perfil padrão para toque e suporte a gamepad Bluetooth/USB via SDL.

- O overlay deve enviar eventos para o backend de input do runtime, não escrever diretamente em memória guest.

## Vulkan e PanVK

O arquivo recebido contém apenas `libvulkan_panfrost.so`, além de licença Mesa e metadados. Ele é ELF AArch64 e declara dependências como `libhardware.so`, `libnativewindow.so`, `libsync.so`, `libdrm.so` e `libc++_shared.so`.

O `meta.json` informa `minApi: 36`; portanto, não é seguro declarar esse binário como compatível com Android 13/14/15 sem validação adicional. O APK deve:

1. Usar primeiro o Vulkan loader/driver do sistema.

1. Detectar GPU, API, extensões e limites via `vkGetPhysicalDeviceProperties2`.

1. Exibir um diagnóstico de driver no menu técnico.

1. Permitir uma opção experimental de biblioteca externa somente em ambiente autorizado/root/custom ROM, sem substituir o loader do sistema automaticamente.

1. Desabilitar PanVK quando API, ABI, dependências ou extensões não conferirem.

1. Manter fallback para Vulkan do sistema e, se necessário, modo de compatibilidade sem overlay avançado.

## Otimizações Mali-G57

- Preferir `arm64-v8a` e compilar o código recompilado com otimizações ARMv8-A conservadoras.

- Não usar instruções exclusivas de Adreno.

- Limitar precisão e variantes SPIR-V conforme os limites reportados pelo driver.

- Cache persistente de shaders por versão do driver e configuração gráfica.

- Evitar recompilação de pipeline durante partida.

- Frame pacing com fila FIFO/MAILBOX apenas quando suportado.

- Reduzir resolução interna e filtros antes de reduzir a superfície Android.

- Coletar telemetria opcional de GPU, memória, temperatura e quedas de frame sem bloquear o jogo.

## Estado atual

- O XEX já foi analisado e o C++ recompilado foi gerado.

- Os 272 arquivos C++ recompilados já passaram pela etapa de compilação host.

- O link host ainda está sendo concluído com os stubs oficiais XUsbcam do ReXGlue.

- O APK Android ainda não está pronto.

