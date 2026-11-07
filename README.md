# fsh

`fsh`, un petit shell avec des boucles `for` développé dans le cadre du projet SY5.

Le fonctionnement interne (parsing, validation, structures de commandes, gestion des signaux, debugger) est détaillé dans [`ARCHITECTURE.md`](ARCHITECTURE.md).

## Dépendances

- `gcc`
- `make`
- `libreadline` (en-têtes de développement, ex. paquet `libreadline-dev` sous Debian/Ubuntu)

## Compilation

```bash
make
```

Cela génère l'exécutable `fsh` à la racine du projet, à partir des sources de `src/` compilées dans `build/`.

Pour nettoyer les fichiers compilés :

```bash
make clean
```

Pour tout recompiler depuis zéro :

```bash
make re
```

### Compilation avec logs de debug

Le projet dispose d'un système de logs contrôlé par deux macros, `LOG_LEVEL` et `LOG_TRIGGER` (voir `ARCHITECTURE.md` pour le détail). Plusieurs cibles `make` permettent de les activer :

```bash
make debug        # active DEBUG_CALL, sans logs
make info          # logs de niveau LOG_INFO
make warning       # logs de niveau LOG_WARNING
make error         # logs de niveau LOG_ERROR
make max_info      # logs <= LOG_INFO
make max_warning   # logs <= LOG_WARNING
make max_error     # logs <= LOG_ERROR
```

## Exécution

```bash
./fsh
```

Le shell affiche un prompt et lit les commandes saisies au clavier. Pour quitter, utiliser la commande intégrée `exit` ou `Ctrl-D`.
