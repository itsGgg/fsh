# Explication du Fonctionnement du Programme

Ce programme traite les lignes de commande saisies par l'utilisateur en suivant une série d'étapes.

---

## Étapes principales :

1. **Transformation de la ligne en tokens :**
   - La ligne saisie par l'utilisateur est d'abord divisée en une série de tokens, chacun représentant un élément distinct de la commande (comme des redirections, des arguments ou des opérateurs).

2. **Validation des tokens :**
   - Cette série de tokens est ensuite donnée à un validateur pour vérifier que la ligne est correctement formée.
   - Le validateur vérifie la syntaxe et la structure logique de la commande.

3. **Conversion en structure de commande :**
   - Une fois validée, la ligne est convertie en une structure interne qui décrit le type de commande qu'elle représente.
   - Cette structure permet au programme d'exécuter la commande de la bonne manière.

---

# Structures pour la Gestion des Types de Commandes

Le code utilise plusieurs structures (définies dans `command.h`) pour gérer différents types de commandes.

---

### `command_t`

La structure `command_t` représente une commande générique.

- **Attributs :**
   - `command_type_t type` : Type de la commande
   - `union {`
      - `simple_command_t *simple`    : une commande simple
      - `pipeline_command_t *pipeline` : une commande pipeline
      - `structured_command_t *structured` : une commande structurée

---

### `structured_sequence_t`

Les commandes structurées sont représentées par une structure `structured_sequence_t` contenant :

- **Attributs :**
   - `command_condition_t condition` : Condition de la structure, pipeline exclu
   - `union {`
      - `structured_sequence_t *seq_cond` : une séquence de commandes
      - `structured_if_t *if_cond` : une structure conditionnelle `if`
      - `structured_if_else_t *if_else_cond` : une structure conditionnelle `if-else`
      - `structured_for_t *for_cond` : une boucle `for`

---

### `simple_command_t` : Commande simple

Les redirections sont également gérées par cette structure :

- **Attributs :**
   - `int argc` : le nombre d'arguments de la commande.
   - `char **args` : les arguments de la commande.
   - `int nb_redir` : le nombre de redirections associées à la commande.
   - `redirection_t **redir` : un tableau dynamique de pointeurs vers des structures `redirection_t`.

---

### `redirection_t` (définie dans `redirection.h`)

Cette structure contient :

- **Attributs :**
   - `int type` : le type de redirection, représenté par une valeur `enum` (`NO_REDIRECT`, `REDIRECT_IN`, `REDIRECT_OUT`, etc.).
   - `char *file` : le nom du fichier cible de la redirection

---

### `pipeline_command_t` : Pipeline

- **Attributs :**
   - `int nb_commands` : Nombre de commandes simples.
   - `simple_command_t **args` : Tableau de commandes simples.

---

### `structured_sequence_t` : Commande structurée

- **Attributs :**
   - `int nb_commands` : Nombre de commandes.
   - `command_t **commands` : Tableau des commandes à exécuter dans l'ordre.

---

### `structured_if_t` : Commande `if` sans `else`

- **Attributs :**
   - `pipeline_command_t *test` : Le test, représenté par un pipeline.
   - `structured_sequence_t *commands` : Commande structurée à exécuter si le test réussit.

---

### `structured_if_else_t` : Commande `if` avec `else`

- **Attributs :**
   - `pipeline_command_t *test` : Le test, représenté par un pipeline.
   - `structured_sequence_t *if_commands` : Commande structurée à exécuter en cas de réussite.
   - `structured_sequence_t *else_commands` : Commande structurée à exécuter en cas d'échec.

---

### `structured_for_t` : Commande `for`

- **Attributs :**
   - `char var` : Variable de boucle pour la commande `for`.
   - `char *rep` : Répertoire cible où la boucle `for` s'exécute.
   - `int nb_options` : Nombre d'options spécifiées pour la boucle.
   - `structured_for_option_t **options` : Tableau des options spécifiques associées à la boucle.
   - `structured_sequence_t *commands` : Commande structurée à exécuter à chaque itération de la boucle.

---

### `structured_for_option_t` : Option des boucles for

- **Attributs :**
   - `char option` : caractère représentant une option unique
   - `char *arg` : argument unique associé à l'option s'il existe

---

# Algorithme de Validation de Commande

L'algorithme effectue une recherche récursive pour vérifier la validité de la commande entrée.

---

## Vérification de la syntaxe

1. La commande **ne doit pas commencer** par `|`.
2. Aucun `|` **ne doit être suivi immédiatement** d'un autre `|`.
3. Les accolades `{` et `}` doivent être correctement équilibrées et positionnées.

---

## Vérification du contenu

1. La commande **ne doit pas commencer par une redirection**.
2. En cas de présence de `;` :
   - Extraire la commande entre deux `;` ou jusqu'à la fin.
   - Vérifier récursivement si cette commande est correcte.
3. En cas de présence de `for` en premier token :
   - Vérifier récursivement si la commande est bien formée.
4. En cas de présence de `if` en premier token :
   - Vérifier récursivement si la commande est bien formée.
5. En cas de présence de redirection :
   - Le token suivant **ne doit pas être une autre redirection**.

---

## Vérification des boucles `for`

1. Le **premier token** doit être `for`.
2. Le **deuxième token** doit être composé uniquement de lettres.
3. Le **troisième token** doit être `in`.
4. Si le token suivant **n'est pas** `{`, vérifier si l'option est valide.
5. Extraire la commande entre les accolades `{}` et vérifier récursivement son contenu.

---

## Vérification des conditions `if`

1. Le **premier token** doit être `if`.
2. Extraire le pipeline avant `{` et vérifier si le pipeline est bien formé.
3. Extraire la commande entre les accolades `{}` et vérifier récursivement sa validité.
4. Si une structure `else` est présente après `}`, extraire la commande suivante entre accolades et vérifier récursivement.

---

## Vérification du pipeline

1. Le pipeline **ne doit pas contenir** :
   - `;`
   - `for`
   - `if`
2. Une commande **doit obligatoirement suivre** un `|`.
3. Un fichier **doit obligatoirement suivre** une redirection.

---

# Gestion des Signaux

Le fsh ignore les signaux `SIGINT` et `SIGTERM`. Ces signaux, toutefois, ne sont pas ignorés par les processus dérivés du fsh.

Le fsh utilise une variable `interrupted` pour gérer les signaux `SIGINT` et `SIGTERM`.

Dans la fonction `execute_external`, le processus fils appelle `reset_signals_to_default()` pour rétablir le comportement par défaut de tous les signaux. Ainsi, lorsque celui-ci exécute une commande et qu'il est interrompu, le père pose la variable `interrupted` à 1 et renvoie le code associé au signal l'ayant interrompu.

Si `interrupted` est égal à 1, et que `last_status` > 255 alors on affiche `[SIG]` au lieu de `last_status` (`prompt.c`).

De plus, les signaux `SIGINT` et `SIGTERM` s'appliquent aussi aux séquences de commandes. Cela est géré dans `execute_structured_sequence` :

- Si `SIGINT` est reçu, l'exécution de la séquence est interrompue, ce qui signifie que l'ensemble des commandes sont interrompues.
- Si `SIGTERM` est reçu, seule la commande en cours d'exécution de la séquence est interrompue.

---

# Debugger

Le debugger de `fsh` permet d'afficher des messages de debug à différents niveaux de gravité. Il est contrôlé par deux macros : `LOG_LEVEL` et `LOG_TRIGGER`. Pour les afficher, il faut compiler le projet en rajoutant à `make` un des arguments suivants :

- `debug`
- `info`
- `warning`
- `error`
- `max_debug`
- `max_info`
- `max_warning`
- `max_error`

---

## Niveaux de gravité

Le debugger supporte trois niveaux de gravité :

- `LOG_INFO` : Messages d'information généraux.
- `LOG_WARNING` : Messages d'avertissement pour des situations potentiellement problématiques.
- `LOG_ERROR` : Messages d'erreur pour des erreurs critiques.

---

## Macros de contrôle

- `LOG_LEVEL` : Définit le niveau de gravité minimum pour que les messages soient affichés. Par exemple, si `LOG_LEVEL` est défini à `LOG_WARNING`, seuls les messages de niveau `LOG_WARNING` et `LOG_ERROR` seront affichés.
- `LOG_TRIGGER` : Définit le mode de déclenchement du debugger. Il peut prendre deux valeurs :
    - `LOG_UNIQ` : Seuls les messages dont le niveau de gravité est **exactement égal** à `LOG_LEVEL` sont affichés.
    - `LOG_MAX` : Tous les messages dont le niveau de gravité est **inférieur ou égal** à `LOG_LEVEL` sont affichés.

---

## Fonctionnement

La fonction `log_message` est appelée pour afficher les messages de debug. Elle prend en arguments le niveau de gravité, le message, le fichier source et le numéro de ligne. Elle vérifie si le message doit être affiché en fonction des valeurs de `LOG_LEVEL` et `LOG_TRIGGER`. Si le message est trop long, il est tronqué et un message d'avertissement est affiché.

---

## Exception

La macro `DEBUG_CALL` est active seulement lorsque `LOG_LEVEL` est défini sur `NO_LOG`. Inversement, les messages de log ne seront affichés que si `LOG_LEVEL` est défini sur un niveau supérieur à `NO_LOG`. Ainsi, `DEBUG_CALL` et les messages de log s'excluent mutuellement :

- Si `LOG_LEVEL` est `NO_LOG` : Les appels `DEBUG_CALL` sont actifs, mais les messages de log ne seront pas affichés.
- Si `LOG_LEVEL` n'est pas `NO_LOG` : Les messages de log seront affichés, mais les appels `DEBUG_CALL` ne produiront aucun effet visible.
