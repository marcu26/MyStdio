# Tema 1 pso - Biblioteca stdio

Din makefile-uri rezulta doar bibliotecile.

## Structura:

- File descriptor : descriptoru de fisier pentru windows/ Handle pentru linux
- Buffer: buffer-ul in care vom scrie si in care vom citi din fisier
- BufferCursor: cursorul bufferului (retine indexul din buffer)
- LastOperation: pentru a retine ultima operatie facuta
- BytesRead: numarul de bytes cititi din fisier
- IsOpenForAppend: flag pentru a seta cursorul la final de fisier
- pid/process: pentru Linux tinem id-ul procesului, la windows informatiile procesului
- Flags: flag pentru a retine modul in care a fost deschis fisierul
- EOF: flag care va fi setat pe -1 in momentul in care ajungem la finalul fisierului la read
- IsError: flag care va fi setat cand exista o eroare la apelurile de sistem



Link git:
https://github.com/marcu26/tema1_pso
