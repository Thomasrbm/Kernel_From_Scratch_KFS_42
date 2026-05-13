# Kernel From Scratch — KFS (42)

> Un mini-noyau x86 32 bits écrit **from scratch**, démarré via **GRUB** (Multiboot 2), installant sa propre **GDT** et offrant un shell interactif (`kash`) avec affichage VGA texte, gestion clavier PS/2 et plusieurs écrans virtuels.

Projet réalisé dans le cadre du cursus **42 — Kernel From Scratch**.

---

## Table des matières

1. [Aperçu](#aperçu)
2. [Fonctionnalités](#fonctionnalités)
3. [Stack technique](#stack-technique)
4. [Arborescence du projet](#arborescence-du-projet)
5. [Pré-requis](#pré-requis)
6. [Compilation et exécution](#compilation-et-exécution)
7. [Cibles du Makefile](#cibles-du-makefile)
8. [Séquence de démarrage](#séquence-de-démarrage)
9. [GDT — Global Descriptor Table](#gdt--global-descriptor-table)
10. [Carte mémoire](#carte-mémoire)
11. [Shell `kash`](#shell-kash)
12. [Sous-système d'affichage VGA](#sous-système-daffichage-vga)
13. [Entrées clavier (PS/2)](#entrées-clavier-ps2)
14. [Détails de bas niveau](#détails-de-bas-niveau)
15. [Dépannage](#dépannage)
16. [Roadmap](#roadmap)
17. [Références](#références)
18. [Licence](#licence)

---

## Aperçu

Ce dépôt contient un noyau **bare-metal** ciblant l'architecture **x86 (i386, mode protégé 32 bits)**. Il est conçu pour être chargé par **GRUB 2** via l'en-tête **Multiboot 2**, configure manuellement sa **GDT** (Global Descriptor Table), puis donne la main à un shell minimaliste, `kash`, qui dessine dans la mémoire vidéo VGA en mode texte (`0xB8000`) et lit les frappes clavier directement depuis le contrôleur PS/2 (`port 0x60`).

Le code est volontairement réduit à l'essentiel et n'utilise **aucune bibliothèque standard** : pas de `libc`, pas de `libgcc`, pas de runtime — uniquement ce qui est explicitement réimplémenté dans `srcs/`.

---

## Fonctionnalités

- **Bootloader-friendly** : header Multiboot 2 conforme à la spécification GRUB.
- **GDT 32 bits** avec 7 segments :
  - Null descriptor
  - Kernel `code` / `data` / `stack` (ring 0)
  - User `code` / `data` / `stack` (ring 3)
- **Relocalisation de la GDT** à l'adresse physique fixe `0x00000800`.
- **Driver VGA texte** 80 × 25 avec gestion des couleurs (palette 16 couleurs).
- **Backbuffer scrollable** de 200 lignes par écran, mis à jour hors-écran puis blit en VGA.
- **3 écrans virtuels indépendants** (F1 / F2 / F3) avec leur propre curseur et leur propre historique.
- **Curseur matériel** (CRTC) synchronisé avec la position logique.
- **Driver clavier PS/2** avec table de scancodes US, gestion `make/break`, touches fléchées et fonctions.
- **Shell `kash`** avec son `printk` variadique (`%d`, `%c`, `%s`).
- **Commandes intégrées** : `help`, `clear`, `reboot`, `poweroff`, `hlt`.
- **HexDump** intégré pour visualiser la GDT installée en mémoire.

---

## Stack technique

| Composant | Outil |
|---|---|
| Langage système | **C** (freestanding, `-m32`) |
| Assembleur | **NASM** (syntaxe Intel, `elf32`) |
| Linker | **GNU ld** (script `linker.ld`) |
| Bootloader | **GRUB 2** (Multiboot 2) |
| Émulation | **QEMU** (`qemu-system-i386`) |
| Image disque | **ISO 9660** via `grub-mkrescue` |

---

## Arborescence du projet

```
Kernel_From_Scratch_KFS_42/
├── Makefile                  # Build / run / clean
├── Readme                    # Notes brutes de dev (legacy)
├── README.md                 # Ce document
├── .gitignore
│
├── srcs/                     # Sources du noyau
│   ├── boot/
│   │   ├── header.s          # En-tête Multiboot 2
│   │   └── boot.s            # Point d'entrée `start`, setup stack, appel C
│   │
│   ├── gdt/
│   │   ├── gdt.h             # Structures GDT (packed)
│   │   ├── gdt.c             # set_gdt_entry / gdt_init
│   │   ├── gdt_flush.s       # lgdt + far jump pour activer la nouvelle GDT
│   │   └── hexadump.c        # DumpHex (vérification visuelle de la GDT)
│   │
│   ├── shell/
│   │   ├── handle_shell.h    # API publique du shell + ports I/O inline
│   │   ├── handle_shell.c    # Boucle principale + init backbuffer
│   │   ├── vga.c             # printk / refresh_g_screen / backbuffer
│   │   ├── shell_utils.c     # newline / strcmpk / print_number / etc.
│   │   ├── process_key.c     # Dispatch scancodes → actions
│   │   └── scancode.c        # Table scancode → ASCII
│   │
│   ├── cmds/
│   │   ├── cmds.h
│   │   ├── help.c            # cmd_help
│   │   ├── clear.c           # cmd_clear
│   │   ├── reboot.c          # cmd_reboot  (8042 reset)
│   │   ├── poweroff.c        # cmd_poweroff (ACPI shutdown QEMU/Bochs)
│   │   └── hlt.c             # cmd_halt   (cli ; hlt)
│   │
│   └── utils/
│       ├── utils.h
│       └── ft_memcpy.c
│
├── targets/
│   ├── linker.ld             # Script linker (point d'entrée = 1 MiB)
│   └── iso/
│       └── boot/grub/grub.cfg
│
├── DOC/                      # Notes et théorie
│   ├── Theorie/              # GdT Intel, alignement, Grub
│   ├── Explications/         # Vulgarisation interne
│   └── tutos/
│
└── distro/x86_64/            # Sortie de build (kernel.bin, kernel.iso)
```

---

## Pré-requis

Sur une distribution **Debian / Ubuntu** :

```bash
sudo apt update
sudo apt install -y \
    nasm \
    gcc \
    gcc-multilib \
    binutils \
    make \
    xorriso \
    grub-pc-bin \
    grub-common \
    qemu-system-x86
```

Sur **Arch Linux** :

```bash
sudo pacman -S nasm gcc lib32-gcc-libs binutils make xorriso grub qemu-system-x86
```

> ⚠️ `gcc-multilib` est indispensable : la cible étant `-m32`, GCC doit pouvoir produire du code i386 même sur un host x86_64.

---

## Compilation et exécution

```bash
# 1. Compilation (assemble, compile, link, génère l'ISO)
make

# 2. Lancement dans QEMU
make run

# 3. Nettoyage complet
make fclean

# 4. Recompilation complète depuis zéro
make re
```

Après `make`, l'image bootable se trouve dans :

```
distro/x86_64/kernel.iso
```

Elle peut être :
- bootée dans **QEMU** (`make run`),
- gravée sur une clé USB pour boot **réel** (à vos risques),
- bootée dans **VirtualBox** / **VMware** en tant qu'image CD/DVD.

---

## Cibles du Makefile

| Cible | Effet |
|---|---|
| `build-x86_64` *(défaut)* | Compile sources, link, génère `kernel.bin`, encapsule dans `kernel.iso` |
| `run` | Lance `qemu-system-i386 -cdrom $(ISO)` dans un environnement nettoyé (`env -i`) |
| `fclean` | Supprime `objs/`, `distro/` et `targets/iso/boot/kernel.bin` |
| `re` | `fclean` + `build-x86_64` |

### Flags de compilation

```make
CFLAGS := -m32 -ffreestanding -fno-builtin -fno-exceptions \
          -fno-stack-protector -nostdlib -nodefaultlibs
```

| Flag | Pourquoi |
|---|---|
| `-m32` | Code i386 32 bits, mode protégé |
| `-ffreestanding` | Pas de `libc`, pas de `main`, pas d'hypothèses runtime |
| `-fno-builtin` | GCC ne remplace pas `memcpy`/`memset` par ses intrinsics |
| `-fno-exceptions` | Pas de stack unwinding C++ |
| `-fno-stack-protector` | Pas de canary (pas de `__stack_chk_*` à fournir) |
| `-nostdlib`, `-nodefaultlibs` | Aucun lien implicite contre la libc |

### Liaison

```bash
ld -m elf_i386 -n -o kernel.bin -T targets/linker.ld $(OBJS)
```

Le script `linker.ld` place le noyau à **1 MiB** (`. = 1M`) en mémoire physique, conformément à la convention Multiboot.

---

## Séquence de démarrage

```
 ┌──────────────┐
 │  BIOS / UEFI │
 └──────┬───────┘
        │  POST → charge le secteur de boot
        ▼
 ┌──────────────┐
 │     GRUB 2   │  ← lit grub.cfg, trouve `multiboot2 /boot/kernel.bin`
 └──────┬───────┘
        │  vérifie la signature Multiboot 2 dans .multiboot_header
        │  charge le binaire à 0x100000 (= 1 MiB)
        │  saute à `start`
        ▼
 ┌────────────────────────────────────┐
 │  boot.s  →  start:                 │
 │    mov esp, stack_top              │  ← installe la pile (16 KiB en .bss)
 │    call gdt_init                   │  ← installe la GDT custom
 │    call handle_shell               │  ← passe la main au shell
 │    hlt                             │
 └────────────────────────────────────┘
```

### En-tête Multiboot 2 (`srcs/boot/header.s`)

```asm
header_start:
    dd 0xe85250d6                                   ; magic Multiboot 2
    dd 0                                            ; architecture (0 = i386)
    dd header_end - header_start                    ; longueur
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum
    dw 0, dw 0, dd 8                                ; end tag
header_end:
```

GRUB refuse de booter si le magic, l'architecture, la longueur **et** le checksum (somme nulle modulo 2³²) ne sont pas cohérents.

---

## GDT — Global Descriptor Table

La GDT décrit les **segments mémoire** vus par le CPU et leurs **niveaux de privilège** (DPL). Bien qu'on utilise un modèle **flat** (chaque segment couvre toute la mémoire, base = 0, limite = 4 GiB), la GDT reste indispensable car le CPU exige des sélecteurs de segments valides pour `cs`, `ds`, `ss`, etc.

### Disposition installée

| # | Sélecteur | Type             | Base | Limite  | Access | Flags | DPL |
|---|-----------|------------------|------|---------|--------|-------|-----|
| 0 | `0x00`    | Null descriptor  | 0    | 0       | `0x00` | `0x0` | —   |
| 1 | `0x08`    | Kernel code      | 0    | `0xFFFFF` (×4 KiB = 4 GiB) | `0x9A` | `0xC` | 0 |
| 2 | `0x10`    | Kernel data      | 0    | `0xFFFFF` | `0x92` | `0xC` | 0 |
| 3 | `0x18`    | Kernel stack     | 0    | `0xFFFFF` | `0x92` | `0xC` | 0 |
| 4 | `0x20`    | User code        | 0    | `0xFFFFF` | `0xFA` | `0xC` | 3 |
| 5 | `0x28`    | User data        | 0    | `0xFFFFF` | `0xF2` | `0xC` | 3 |
| 6 | `0x30`    | User stack       | 0    | `0xFFFFF` | `0xF2` | `0xC` | 3 |

### Décomposition du byte `access`

```
bit 7   bit 6   bit 5   bit 4   bit 3   bit 2   bit 1   bit 0
  P      DPL     DPL     S      Type    Type    Type    Type
|__P__|_____DPL_____|__S__|_____________type____________|
```

- **P** : segment présent (1) sinon `#NP` à l'usage.
- **DPL** : ring autorisé (0 = kernel, 3 = user).
- **S** : 1 = segment code/data, 0 = segment système (TSS, LDT…).
- **Type** : pour les segments code/data, contient `E/W/A` (execute, writable, accessed) + bit 3 (1 = code, 0 = data).

Exemples :
- `0x9A` = `1001 1010` = présent, DPL 0, code, exécutable, lisible.
- `0x92` = `1001 0010` = présent, DPL 0, data, inscriptible.
- `0xFA` / `0xF2` = mêmes flags, mais DPL = 3 (user).

### Décomposition du nibble `limit_flags`

```
bit 7   bit 6   bit 5   bit 4   bit 3   bit 2   bit 1   bit 0
  G      DB      L      AVL    L19     L18     L17     L16
|________flags________|        |____limite 16..19______|
```

- **G** : granularité (1 = la limite est en pages 4 KiB → max 4 GiB).
- **DB** : 1 = segment 32 bits.
- **L** : 0 = pas en mode long (64 bits).
- **AVL** : libre pour le système.

`0xC` = `1100` → G = 1, DB = 1, L = 0, AVL = 0.

### Activation (`gdt_flush.s`)

```asm
gdt_flush:
    mov eax, [esp+4]      ; pointeur sur la t_gdt_ptr
    lgdt [eax]             ; charge GDTR

    mov ax, 0x10           ; sélecteur kernel data (index 2 × 8)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:flush         ; far jump → recharge CS = kernel code
flush:
    ret
```

Le **far jump** est obligatoire : `lgdt` ne touche pas `CS`. Il faut un saut inter-segment pour que le CPU recharge `CS` avec la nouvelle valeur (`0x08` = sélecteur kernel code).

### Adresse fixe `0x00000800`

```c
g_gdt_ptr.base  = 0x00000800;
g_gdt_ptr.limit = sizeof(g_gdt) - 1;
ft_memcpy((void*)0x00000800, g_gdt, sizeof(g_gdt));
gdt_flush(&g_gdt_ptr);
```

La GDT est **recopiée** à l'adresse physique `0x800` pour qu'elle vive à un endroit prévisible, hors du `.data` du noyau. C'est aussi à cet endroit que `DumpHex((void*)0x800, sizeof(g_gdt))` la dessine au boot pour vérification visuelle.

---

## Carte mémoire

| Adresse | Contenu |
|---|---|
| `0x000000` – `0x0007FF` | Zone basse — laissée intacte (IVT BIOS, BDA…) |
| `0x000800` – `0x000837` | **GDT** (7 entrées × 8 octets = 56 octets) |
| `0x000838` – `0x09FFFF` | Libre |
| `0x0B8000` – `0x0BFFFF` | **VGA text buffer** (80 × 25 × 2 octets) |
| `0x100000` – …          | Noyau (chargé par GRUB à 1 MiB) |
| Stack noyau (`.bss`)    | 16 KiB (`4096 * 4`), grandit vers le bas |

---

## Shell `kash`

Au démarrage, l'écran affiche :

```
============== Welcome to Kash ==============

                 :::      ::::::::
               :+:      :+:    :+:
             +:+ +:+         +:+
           +#+  +:+       +#+
          +#+#+#+#+#+   +#+
               #+#    #+#
              ###    ##########

============== screen (0) ==================

kash>
```

Puis le `DumpHex` de la GDT installée à `0x800` est imprimé, suivi d'un nouveau prompt.

### Commandes disponibles

| Commande | Effet | Implémentation |
|---|---|---|
| `help` | Affiche la liste des commandes | `cmds/help.c` |
| `clear` | Vide le backbuffer de l'écran courant et réimprime le welcome | `cmds/clear.c` |
| `reboot` | Reset CPU via le contrôleur clavier 8042 (`outb 0x64, 0xFE`) | `cmds/reboot.c` |
| `poweroff` | Shutdown ACPI (`outw 0x604, 0x2000`) — fonctionne sous **QEMU**/Bochs | `cmds/poweroff.c` |
| `hlt` | `cli ; hlt` — arrête le CPU jusqu'à interruption | `cmds/hlt.c` |

### Écrans virtuels

| Touche | Action |
|---|---|
| **F1** (scancode `0x3B`) | Bascule sur l'écran 0 |
| **F2** (scancode `0x3C`) | Bascule sur l'écran 1 (welcome au premier appel) |
| **F3** (scancode `0x3D`) | Bascule sur l'écran 2 (welcome au premier appel) |

Chaque écran possède son propre `g_backbuffer`, son propre `g_cursor_line/col` et son propre `g_view_offset`.

### Navigation et édition

| Touche | Action |
|---|---|
| **←** / **→** | Déplacement du curseur sur la ligne courante (borné par le prompt et la fin de saisie) |
| **↑** / **↓** | Scroll dans l'historique (200 lignes de backbuffer) |
| **Backspace** | Efface le caractère précédent (sans franchir le prompt) |
| **Entrée** | Exécute la commande saisie |

---

## Sous-système d'affichage VGA

### Architecture en double-tampon

```
       printk(...)
            │
            ▼
   ┌─────────────────────┐
   │  g_backbuffer[3]    │  ← BUFFER_LINES (200) × VGA_WIDTH (80) par écran
   │   (in-memory)       │
   └─────────┬───────────┘
             │  refresh_g_screen()
             ▼
   ┌─────────────────────┐
   │   0xB8000 (VGA RAM) │  ← 25 lignes visibles, fenêtre dans le backbuffer
   └─────────────────────┘
```

Chaque cellule fait 16 bits : `(couleur << 8) | char`. La palette définie dans `handle_shell.h` couvre 13 combinaisons (vert clair, cyan, jaune, magenta…), toutes sur fond noir.

### `printk` (variadique freestanding)

Supporte trois formats :
- `%d` — entier signé (réimplémenté via récursion sur `print_number`)
- `%c` — caractère
- `%s` — chaîne

Aucun `<stdio.h>` n'est utilisé. Chaque écriture passe par `backbuffer_fill_char`, qui prend en compte `g_screen` pour cibler le bon écran virtuel.

### Curseur matériel

Le CRTC VGA est piloté via les ports `0x3D4` (index) et `0x3D5` (data), registres 14 (high byte) et 15 (low byte) :

```c
outb(VGA_INDEX_PORT, 14);
outb(VGA_DATA_PORT, pos >> 8);
outb(VGA_INDEX_PORT, 15);
outb(VGA_DATA_PORT, pos & 0xFF);
```

`pos` est calculé à partir de `g_cursor_line - g_view_offset` pour rester cohérent avec le scroll.

---

## Entrées clavier (PS/2)

Le contrôleur PS/2 est interrogé en **polling** (pas encore d'IRQ) :

```c
while (!(inb(KB_STATUS_PORT) & 0x01))   // attend qu'un byte soit dispo
    ;
scancode = inb(KB_DATA_PORT);           // lit le scancode
process_key(scancode);
```

| Port | Rôle |
|---|---|
| `0x60` | Data port (scancode) |
| `0x64` | Status port (bit 0 = output buffer full) |

- Les **make codes** (appui) ont le bit 7 à 0.
- Les **break codes** (relâchement) ont le bit 7 à 1 (`KB_RELEASE_MASK = 0x80`) — ignorés.
- La table `g_scancode_keyboard[128]` (layout US) traduit le scancode en ASCII.

---

## Détails de bas niveau

### Wrappers I/O inline (`handle_shell.h`)

```c
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
```

L'attribut `static inline` permet l'inlining sans symbole exporté, indispensable en mode freestanding.

### Pile noyau

```asm
section .bss
align 4
    stack_bottom:
        resb 4096 * 4     ; 16 KiB
    stack_top:
```

Initialisée par `mov esp, stack_top` dans `start`. La pile **croît vers le bas**, d'où l'usage de `stack_top` comme valeur initiale d'`ESP`.

### Notes de sécurité ELF

Chaque fichier `.s` se termine par :

```asm
section .note.GNU-stack noalloc noexec nowrite progbits
```

Cela indique au linker que la pile **ne doit pas être exécutable** — sans cette note, `ld` peut marquer la pile `RWX` par défaut.

---

## Dépannage

| Symptôme | Cause probable | Solution |
|---|---|---|
| `grub-mkrescue: command not found` | `grub-pc-bin` ou `grub-common` manquant | `sudo apt install grub-pc-bin grub-common xorriso` |
| `Error: invalid Multiboot header` au boot | Checksum ou magic incorrect dans `header.s` | Vérifier `dd 0xe85250d6` et la formule du checksum |
| Écran noir après `make run` | Le noyau s'est arrêté avant tout `printk` | Lancer QEMU avec `-d int,cpu_reset` pour voir les exceptions |
| `Triple fault` immédiat | GDT mal alignée ou sélecteur `CS` invalide | Vérifier que l'entrée 1 est bien à `0x9A`/`0xC` et que le far jump cible `0x08` |
| `make` échoue avec `cannot find -lgcc` | Flags `-nostdlib`/`-nodefaultlibs` mal placés | Vérifier la ligne `CFLAGS` du `Makefile` |
| Caractères imprimés blancs sur fond noir au boot | Le backbuffer n'a pas été initialisé | S'assurer que `init_backbuffer()` est appelé avant `welcome_prompt()` |

---

## Roadmap

Étapes prévues dans la suite du cursus **KFS** :

- [ ] **KFS 3** — Memory management : gestionnaire de pages, paging (PD/PT), `kmalloc`/`kfree`.
- [ ] **KFS 4** — IDT, ISRs, IRQs (PIC 8259), passage clavier en interrupt-driven.
- [ ] **KFS 5** — Multitâche coopératif (TSS, context switch).
- [ ] **KFS 6** — Système de fichiers en mémoire.
- [ ] **KFS 7** — Syscalls + séparation user / kernel space (ring 3 réellement exploité).
- [ ] **KFS 8/9/10** — ELF loader, signaux, modules.

---

## Références

- [OSDev Wiki — Global Descriptor Table](https://wiki.osdev.org/Global_Descriptor_Table)
- [OSDev Wiki — Multiboot 2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)
- [OSDev Wiki — Bare Bones Tutorial](https://wiki.osdev.org/Bare_Bones)
- [OSDev Wiki — PS/2 Keyboard](https://wiki.osdev.org/PS/2_Keyboard)
- [OSDev Wiki — VGA Hardware](https://wiki.osdev.org/VGA_Hardware)
- *Intel® 64 and IA-32 Architectures Software Developer's Manual* — Volume 3 (System Programming Guide), chapitre 3 (Protected-Mode Memory Management).

D'autres notes internes se trouvent dans `DOC/` :
- `DOC/Theorie/` — extraits Intel, alignement, interactions GRUB ↔ GDT.
- `DOC/Explications/` — vulgarisation des structures GDT propres au projet.
- `DOC/tutos/` — liens vers les tutoriels suivis.

---

## Licence

Projet académique réalisé dans le cadre de l'école **42**. Usage pédagogique.
