# Mensajes en español para GNU make.
# Copyright (C) 1996 Free Software Foundation, Inc.
# Max de Mendizábal <max@acer.com.mx>, 1996.
#
msgid ""
msgstr ""
"Project-Id-Version: GNU make 3.74.4\n"
"POT-Creation-Date: 1996-05-22 09:11-0400\n"
"PO-Revision-Date: 1996-11-21 20:13+0200\n"
"Last-Translator: Max de Mendizábal <max@tirania.nuclecu.unam.mx>\n"
"Language-Team: Spanish <es@li.org>\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=ISO-8859-1\n"
"Content-Transfer-Encoding: \n"

#: ar.c:48
#, c-format
msgid "attempt to use unsupported feature: `%s'"
msgstr "Se intentó utilizar una característica no implementada: `%s'"

#: ar.c:142
#, c-format
msgid "Error in lbr$ini_control, %d\n"
msgstr "Error en lbr$ini_control, %d\n"

#: ar.c:147
#, c-format
msgid "Error opening library %s to lookup member %s, %d\n"
msgstr "Error al abrir la biblioteca %s para buscar al elemento %s, %d\n"

#: ar.c:153
#, c-format
msgid "Error looking up module %s in library %s, %d\n"
msgstr "Error al buscar el módulo %s en la biblioteca %s, %d\n"

#: ar.c:159
#, c-format
msgid "Error getting module info, %d\n"
msgstr "Error al intentar obtener la información del módulo, %d\n"

# Sugerencia: touch -> `touch'. sv
# Ok, aceptada. A falta de cursivas... mm
# No veo porqué no se ha de usar 'tocar' en vez de touch em+
# El mensaje además hace referencia a un touch de un objeto dentro
# de una librería, y de eso no se dice nada en el mensaje. Creo que
# hay que tener cuidado con estas cosas.
# El programa para tocar un miembro de un archivo ... , y ahi estamos
# ya en problemas con el dichoso archivo/fichero.
# A ver que se os ocurre em+
# Enrique: touch es un programa del sistema operativo y sirve para
# cambiar la fecha de un programa o archivo. Es decir lo "toca" y
# modifica sus atributos. Por eso preferí no traducirlo.
#: ar.c:244
msgid "touch archive member is not available on VMS"
msgstr "El programa para hacer un `touch' no está disponible en VMS"

#: ar.c:276
#, c-format
msgid "touch: Archive `%s' does not exist"
msgstr "touch: El archivo `%s' no existe"

#: ar.c:279
#, c-format
msgid "touch: `%s' is not a valid archive"
msgstr "touch: `%s' no es un archivo válido"

#: ar.c:282
msgid "touch: "
msgstr "touch: "

#: ar.c:285
#, c-format
msgid "touch: Member `%s' does not exist in `%s'"
msgstr "touch: El miembro `%s' no existe en `%s'"

#: ar.c:291
#, c-format
msgid "touch: Bad return code from ar_member_touch on `%s'"
msgstr "touch: Código de retorno erróneo de ar_member_touch en `%s'"

#: arscan.c:550
msgid " (name might be truncated)"
msgstr " (el nombre puede estar truncado)"

#: arscan.c:552
#, c-format
msgid "  Date %s"
msgstr "  Fecha %s"

#: arscan.c:553
#, c-format
msgid "  uid = %d, gid = %d, mode = 0%o.\n"
msgstr "  uid = %d, gid = %d, modo = 0%o.\n"

#: dir.c:678
msgid ""
"\n"
"# Directories\n"
msgstr ""
"\n"
"# Directorios\n"

#: dir.c:686
#, c-format
msgid "# %s: could not be stat'd.\n"
msgstr "# %s: podría no estar establecido.\n"

# En el K & R aparece inode traducido como nodo-i. ¿qué te parece? sv
# Bien. Me gusta con el guioncito. mm
#: dir.c:689
#, c-format
msgid "# %s (device %d, inode [%d,%d,%d]): could not be opened.\n"
msgstr ""
"# %s (dispositivo %d, nodo-i [%d,%d,%d]): posiblemente no se pueda abrir.\n"

# ¿No sobraría el "posiblemente"?
# Propongo dejarlo en " no se pudo abrir ". sv
# Ok. Es consistente con otras traducciones. mm
#: dir.c:694
#, c-format
msgid "# %s (device %d, inode %d): could not be opened.\n"
msgstr "# %s (dispositivo %d, nodo-i %d): no se pudo abrir.\n"

#: dir.c:709
#, c-format
msgid "# %s (device %d, inode [%d,%d,%d]): "
msgstr "# %s (dispositivo %d, nodo-i [%d,%d,%d]): "

#: dir.c:714
#, c-format
msgid "# %s (device %d, inode %d): "
msgstr "# %s (dispositivo %d, nodo-i %d): "

#: dir.c:718 dir.c:738
msgid "No"
msgstr "No"

#: dir.c:721 dir.c:741
msgid " files, "
msgstr " archivos, "

#: dir.c:723 dir.c:743
msgid "no"
msgstr "no"

#: dir.c:726
msgid " impossibilities"
msgstr " imposibilidades"

# ¿"So far" no era "hasta ahora"? (no me hagas mucho caso) sv
# Si tu traducción es mejor. Aceptada. mm
#: dir.c:730
msgid " so far."
msgstr " hasta ahora."

#: dir.c:746
#, c-format
msgid " impossibilities in %u directories.\n"
msgstr " imposibilidades en %u directorios.\n"

# Creo que eventually sería "finalmente" o algo así. sv
# Si, es finalmente no a veces. Ok. mm
# referencia me parece que no lleva tilde. sv
# No, no lleva acento. Me emocioné con elos. mm
# Creeis de veras que tiene sentido 'finalmente'em+
# yo pondría ser termina autoreferenciando, por
# ejemplo em+
# Mejor lo eliminamos. Es de alguna forma reiterativo e innecesario.
# Lo de `al final' puede ser más confuso. mm
#: expand.c:92 expand.c:97
#, c-format
msgid "Recursive variable `%s' references itself (eventually)"
msgstr "La variable recursiva `%s' se auto-referencia"

# ¿Qué te parece "atención"? Lo hemos usado mucho en otros programas. sv
# Pero que bestia soy. Perdón por el desbarre. mm
#: expand.c:120
#, c-format
msgid "warning: undefined variable `%.*s'"
msgstr "atención: la variable `%.*s' no ha sido definida"

# No me gusta nada ( lo siento )
# ¿ Qué tal : La referencia a una variable está sin terminar em+
# Creo que tienes razón la voz pasiva es muy desagradable. mm.
#: expand.c:223 expand.c:225
msgid "unterminated variable reference"
msgstr "la referencia a la variable está sin terminar"

#: file.c:264
#, c-format
msgid "Commands were specified for file `%s' at %s:%u,"
msgstr "Las instrucciones fueron especificadas para el archivo `%s' en %s:%u,"

# Buscando en las legras implícitas em+
# Utilicé por búsqueda en para evitar el gerundio. mm
#: file.c:270
#, c-format
msgid "Commands for file `%s' were found by implicit rule search,"
msgstr ""
"Las instrucciones para el archivo `%s' se encontraron por búsqueda en reglas "
"implícitas,"

# "now" es "ahora". ¿te lo has comido consciente o inconscientemente? sv
# inconscientemente. mm
#: file.c:274
#, c-format
msgid "but `%s' is now considered the same file as `%s'."
msgstr "pero `%s' se considera ahora como el mismo archivo que `%s'."

# Lo repetiré una y mil veces... :-)
# "To ignore" *no* es ignorar. sv
# Propongo "no serán tenidas en cuenta" a falta de algo mejor.
# Ok. Mi necedad sobrepasa el milenio. mm
#: file.c:278
#, c-format
msgid "Commands for `%s' will be ignored in favor of those for `%s'."
msgstr "Las instrucciones para `%s' no serán tenidas en cuenta en favor de las que están en `%s'."

# Me suena que eso de colon es algo así como punto, dos puntos o punto y coma.
# ¿Podrías comprobarlo? sv
# Es un punto sencillo (.). A ver si te gusta mi propuesta. mm
#: file.c:299
#, c-format
msgid "can't rename single-colon `%s' to double-colon `%s'"
msgstr "no se puede cambiar un punto (.) `%s' por dos puntos (:) `%s'"

# Lo mismo. sv
#: file.c:302
#, c-format
msgid "can't rename double-colon `%s' to single-colon `%s'"
msgstr "no se pueden cambiar dos puntos (:) `%s' por un punto (.) `%s'"

# Propongo un cambio en el tiempo verbal: Se borra el archivo temporal. sv
# Ok. mm
#: file.c:363
#, c-format
msgid "*** Deleting intermediate file `%s'"
msgstr "*** Se borra el archivo temporal `%s'"

#: file.c:523
msgid "# Not a target:"
msgstr "# No es un objetivo:"

# Pondría: es una dependencia de em+
# Si, tienes razón. mm
#: file.c:531
msgid "#  Precious file (dependency of .PRECIOUS)."
msgstr "#  Archivo valioso (es una dependencia de .PRECIOUS)."

# Lo mismo. Y traducir por falso es poco menos que poco exacto.
# Los phony target de make son aquellos que se ejecutan siempre
# sin tener en cuenta si existe un archivo con el nombre del objetivo
# y de si es actual. Propondría incondicional en vez de falso, ya que
# explica exactamente qué es un phony target em+
# No. Incondicional no es una traducción correcta. Que tal si al rato
# se les ocurre hacer una nueva instrucción .INCONDITIONAL.
# Si quieres podríamos pensar en otra traducción como señuelo o algo así. mm
#: file.c:533
msgid "#  Phony target (dependency of .PHONY)."
msgstr "#  objetivo falso (dependencia de .PHONY)."

# FUZZY
# Pondria línea de comandos
# Y creo que no sé exactamente a que se refiere, No creo que haya visto
# este mensaje nunca en make lo marco con FUZZY em+
# Están traduciendo commands como comandos? Son más bien ordenes o
# instrucciones. Por mi parte no hay problema, incluso me gusta más
# comandos, pero es un anglicismo. mm
#: file.c:535
msgid "#  Command-line target."
msgstr "#  Objetivo de línea de instrucciones."

# Habría que entender esto
# Creo que esto es lo que significa. El fuente no es demasiado claro. mm
# Se refiere a que el fichero makefile es uno que encuentra por
# defecto ( en este orden GNUmakefile, Makefile y makefile ) o uno
# que está definido en la variable de entorno MAKEFILE em+
# Así pues tu traducción es completamente equivocada, lo siento.
# Pondría: Fichero por defecto o definido en la variable MAKEFILE em+
# Cierto, lo has entendido bien. Arreglo acorde. mm
#: file.c:537
msgid "#  A default or MAKEFILES makefile."
msgstr "#  Archivo por defecto o definido en la variable MAKEFILE."

# Propongo efectuada en lugar de terminada. sv
# Ok. Es más literal. mm
# Atención ! , es la búsqueda 'de'reglas implícitas em+
# Ok. Ojo, debo pluralizar regla e implícita. Platicarlo con Ulrich. mm
#: file.c:538
#, c-format
msgid "#  Implicit rule search has%s been done.\n"
msgstr "#  La búsqueda de regla(s) implícita(s)%s ha sido efectuada.\n"

#: file.c:539 file.c:564
msgid " not"
msgstr " no"

#: file.c:541
#, c-format
msgid "#  Implicit/static pattern stem: `%s'\n"
msgstr "#  rama del patrón implícita/estática: `%s'\n"

#: file.c:543
msgid "#  File is an intermediate dependency."
msgstr "#  El archivo es una dependencia intermedia."

# Sugerencia: "crea". sv
# Ok. Aunque, viendolo bien, que tal "hace"? mm.
#: file.c:546
msgid "#  Also makes:"
msgstr "#  También hace:"

# ¿y "comprobó"? sv
# Ok. Mejora. mm
# No pondría nunca, sino simplemente 'no se comprobó 'em+
# Si, es lo mismo pero es más español. mm
#: file.c:552
msgid "#  Modification time never checked."
msgstr "#  La fecha de modificación no se comprobó."

#: file.c:554
msgid "#  File does not exist."
msgstr "#  El archivo no existe."

#: file.c:557
#, c-format
msgid "#  Last modified %.24s (%0lx)\n"
msgstr "#  Última modificación %.24s (%0lx)\n"

#: file.c:560
#, c-format
msgid "#  Last modified %.24s (%ld)\n"
msgstr "#  Última modificación %.24s (%ld)\n"

#: file.c:563
#, c-format
msgid "#  File has%s been updated.\n"
msgstr "#  El archivo%s ha sido actualizado.\n"

#: file.c:568
msgid "#  Commands currently running (THIS IS A BUG)."
msgstr "#  Las instrucciones siguen ejecutándose (ESTO ES UN BUG)."

# ¿No sería más bien "las instrucciones de las dependencias"? sv
# Si, que babas soy. Ahora corrijo. mm
#: file.c:571
msgid "#  Dependencies commands running (THIS IS A BUG)."
msgstr "#  Las instrucciones de las dependencias siguen ejecutándose (ESTO ES UN BUG)."

#: file.c:580
msgid "#  Successfully updated."
msgstr "#  Actualizado con éxito."

#: file.c:584
msgid "#  Needs to be updated (-q is set)."
msgstr "#  Necesita ser actualizado (la opción -q está activa)."

#: file.c:587
msgid "#  Failed to be updated."
msgstr "#  Fallo al ser actualizado."

# ## Le he añadido un "¡" con tu permiso.
# Gracias. No lo puse porque no se como poner ese símbolo con el
# iso-accents-mode y luego se me olvidó. Por cierto, como se hace?
# también tengo duda de la interrogación abierta.
#: file.c:590
msgid "#  Invalid value in `update_status' member!"
msgstr "#  ¡Valor inválido en el miembro `update_status'!"

# ## Lo mismo.
#: file.c:597
msgid "#  Invalid value in `command_state' member!"
msgstr "#  ¡Valor inválido en el miembro `command_state'!"

#: file.c:616
msgid ""
"\n"
"# Files"
msgstr ""
"\n"
"# Archivos"

# Esto habría que revisarlo. sv
# En efecto, había que revisarlo, no es número de archivos
# sino que no hay archivos. mm
#: file.c:639
msgid ""
"\n"
"# No files."
msgstr ""
"\n"
"# No hay archivos."

# Sugerencia: hash buckets -> `hash buckets'
# (al menos hasta que sepamos lo que es, creo que me salió algo parecido
# en recode). sv
# Literalmente un hash bucket es un tonel de trozos. En un proceso de
# partición por picadillo (hash) se deben definir "toneles" o "cubetas"
# para guardar allí la información "hasheada". En otras palabras es el
# tamaño de las entradas de índice hash. Claro como el lodo?
# Finalmente, ok. Por ahora no traducimos hasta ponernos de acuerdo.
#: file.c:642
#, c-format
msgid ""
"\n"
"# %u files in %u hash buckets.\n"
msgstr ""
"\n"
"# %u archivos en %u `hash buckets'.\n"

#: file.c:644
#, c-format
msgid "# average %.1f files per bucket, max %u files in one bucket.\n"
msgstr ""
"# promedio de %.1f archivos por `bucket', max %u archivos en un `bucket'.\n"

#: function.c:648
msgid "undefined"
msgstr "indefinido"

#: function.c:657 variable.c:736
msgid "default"
msgstr "por defecto"

#: function.c:660 variable.c:739
msgid "environment"
msgstr "entorno"

#: function.c:663
msgid "file"
msgstr "archivo"

# ¿ crees sinceramente que esto lo puede entender alguien ? em+
# ¿ qué tal ? con preferencia sobre el entorno em+
# No, está en AIX, ahora lo arreglo. mm
#: function.c:666
msgid "environment override"
msgstr "con preferencia sobre el entorno"

# Línea de comandos me parece correcto em+
# Ver arriba. Estoy de acuerdo si así han traducido en otros lados. mm
#: function.c:669 variable.c:748
msgid "command line"
msgstr "línea de instrucciones"

# Habra que compilar esto inmediatamente e instalar
# el .po a ver que es esto em+
# No se puede por ahora pues make no ha sido internacionalizado aún.
# Será una de las primeras cosas que haga en cuanto se pueda. mm
#: function.c:672
msgid "override"
msgstr "sobreposición"

# Lo he cmabiado em+
# Ok. Deberíamos platicarlo con Ulrich. mm
#: function.c:675 variable.c:754
msgid "automatic"
msgstr "automática/o"

#: function.c:1087 function.c:1089
msgid "non-numeric first argument to `word' function"
msgstr "el primer argumento de la función `word' no es numérico"

# Sugerencia: Borrar el "de" en "requiere de ..." sv
# Perdón, siempre se me vá a pesar de haberlo discutido un buen rato. mm
#: function.c:1097 function.c:1100
msgid "the `word' function takes a one-origin index argument"
msgstr "la función `word' requiere un argumento indexado de tipo origen-uno"

#: function.c:1341
#, c-format
msgid "unterminated call to function `%s': missing `%c'"
msgstr "la llamada a la función `%s' no concluyó: falta `%c'"

# Y no sería mejor "Buscando una regla implícita ..."
# Ten en cuenta que este mensaje no parece un mensaje de error, sino más
# de "debug" o de "verbose". sv
# Cierto. mm
#: implicit.c:38
#, c-format
msgid "Looking for an implicit rule for `%s'.\n"
msgstr "Buscando una regla implícita para `%s'.\n"

# Lo mismo.
# Buscando una regla implítita para el miembro del archivo  `%s' em+
# Se me resbaló. mm
#: implicit.c:53
#, c-format
msgid "Looking for archive-member implicit rule for `%s'.\n"
msgstr "Buscando una regla implícita para el miembro del archivo `%s'.\n"

# Pues si ilegal le suena a cárcel a Enrique, "evade" me suena a mí a
# escaparse de la cárcel... (fuga de alcatraz :-)
# te voy a dar yo a tí fuga ... em+
# ¿Qué te parecería "se evita"? sv
# Me gusta evade, pero evita es correcto. Acepto tu sugerencia. mm
# No me gusta se evita, pondría evitando em+
# Ok. Pero a mi paranoia de evitar gerundios dónde la dejas? mm
#: implicit.c:190
#, c-format
msgid "Avoiding implicit rule recursion.%s%s\n"
msgstr "Evitando la recursión en la regla implícita.%s%s\n"

# FUZZY
# Esto me suena fatal. Se a que se refiere, porque lo
# he visto, pero en cristiano no se si podría entenderlo.
# se refiere a las reglas del tipo % , lo pongo FUZZY , tendré
# que pensar un poco en ello em+
# De momento cambio la regla patron por una regla de patron , y
# pondría a toda costa gerundio ( intentando ) em+
# Ok con el gerundio. Pero también tengo que meditarlo. mm
#: implicit.c:326
#, c-format
msgid "Trying pattern rule with stem `%.*s'.\n"
msgstr "Intentando una regla de patrón con la ramificación `%.*s'.\n"

# ## Corrijo la palabra "dependencia". sv
# Cuando aparezca este mensaje tendrá poco menos que sentido
# testimonial. Date cuenta que los argumentos estan cambiados.
# Pon el orden correcto, siempre será más lógico que lo que vaya a
# aparecer tal y como está ahora (Se rechaza la dependencia imposible
#  `%s' `%s )'em+
# Ok. mm
#: implicit.c:365
#, c-format
msgid "Rejecting impossible %s dependency `%s'.\n"
msgstr "Se rechaza la dependencia imposible %s `%s'.\n"

#: implicit.c:366 implicit.c:374
msgid "implicit"
msgstr "implícita"

#: implicit.c:366 implicit.c:374
msgid "rule"
msgstr "regla"

#: implicit.c:373
#, c-format
msgid "Trying %s dependency `%s'.\n"
msgstr "Se intenta la dependencia %s `%s'.\n"

#: implicit.c:393
#, c-format
msgid "Found dependency as `%s'.%s\n"
msgstr "Se encuentra la dependencia como `%s'.%s\n"

#: implicit.c:408
#, c-format
msgid "Looking for a rule with %s file `%s'.\n"
msgstr "Se busca una regla con el archivo %s `%s'.\n"

#: implicit.c:409
msgid "intermediate"
msgstr "intermedia"

#: job.c:190
#, c-format
msgid "*** [%s] Error 0x%x%s"
msgstr "*** [%s] Error 0x%x%s"

# Preferiría mil veces "sin efecto" o algo parecido. sv
# Ok. Habíamos quedado en `no tiene efecto'. mm
#: job.c:190
msgid " (ignored)"
msgstr " (no tiene efecto)"

# Lo mismo. sv
#: job.c:193
#, c-format
msgid "[%s] Error %d (ignored)"
msgstr "[%s] Error %d (no tiene efecto)"

#: job.c:194
#, c-format
msgid "*** [%s] Error %d"
msgstr "*** [%s] Error %d"

# Sugerencia: " (volcado de `core')". sv
# volcado de `core' no significa nada, que te parece si mejor dejamos
# el core dumped, o bien volcado del núcleo o algo así. Por el momento
# no cambio nada. mm
#: job.c:199
msgid " (core dumped)"
msgstr " (core dumped) [Núcleo vaciado a un archivo]"

# Recibida o capturada, pero por Dios , no pongas
# 'se tiene' . En vez de hijo pon proceso hijo , y quita lo
# que tienes entre paréntesis em+
# Lo del paréntesis era para escoger. mm
#: job.c:234
#, c-format
msgid "Got a SIGCHLD; %d unreaped children.\n"
msgstr "Recibí una señal SIGCHLD; %d proceso hijo descarriado.\n"

# ¿Y job -> trabajos? sv
# Si, pero no hablas de una computadora multi-trabajos sino multi-tareas.
# Por eso elegí tareas. Lo platicamos con más calma? mm
#: job.c:265
msgid "*** Waiting for unfinished jobs...."
msgstr "*** Se espera a que terminen otras tareas...."

# ¿De verdad se dice "vivo"? Si es un proceso, se me ocurre "activo". sv
# Me gusta lo de activo. mm
#: job.c:290
#, c-format
msgid "Live child 0x%08lx PID %d%s\n"
msgstr "Hijo activo 0x%08lx PID %d%s\n"

#: job.c:292 job.c:427 job.c:514 job.c:919
msgid " (remote)"
msgstr " (remoto)"

# Protestar al autor. No hay forma de poner trabajo(s) desconocido(s)
# con coherencia. sv
# Si, si no tienen identificado a su padre. Ni modo. mm
# Pongo /s em+
# Ok. mm
#: job.c:414
#, c-format
msgid "Unknown%s job %d"
msgstr "Trabajo%s desconocido/s %d"

#: job.c:414
msgid " remote"
msgstr " remoto"

#: job.c:419
#, c-format
msgid "%s finished."
msgstr "%s terminado."

# Lo mismo, pon Proceso hijo, y quita lo que hay
# entre paréntesis em+
#: job.c:424
#, c-format
msgid "Reaping %s child 0x%08lx PID %d%s\n"
msgstr "Proceso hijo %s descarriado 0x%08lx PID %d%s\n"

#: job.c:425
msgid "losing"
msgstr "se pierde"

#: job.c:425
msgid "winning"
msgstr "se gana"

# Proceso hijo em+
# Ok.
#: job.c:512
#, c-format
msgid "Removing child 0x%08lx PID %d%s from chain.\n"
msgstr "Se elimina al proceso hijo 0x%08lx PID %d%s de la cadena.\n"

# Proceso hijo em+
# Ok.
#: job.c:917
#, c-format
msgid "Putting child 0x%08lx PID %05d%s on the chain.\n"
msgstr "Se pone al proceso hijo 0x%08lx PID %05d%s en la cadena.\n"

#: job.c:1140
msgid "cannot enforce load limits on this operating system"
msgstr "no se pueden forzar los límites de carga en este sistema operativo"

#: job.c:1142
msgid "cannot enforce load limit: "
msgstr "no se puede forzar la carga límite: "

#: job.c:1244
#, c-format
msgid "internal error: `%s' command_state %d in child_handler"
msgstr "error interno: `%s' command_state %d en el child_handler"

# "en sustituto" me suena muy raro. Propongo: "en su lugar" en su lugar :-) sv
# Ok. mm
#: job.c:1350
#, c-format
msgid "Executing %s instead\n"
msgstr "En su lugar, se ejecuta %s\n"

#: job.c:1381
#, c-format
msgid "Error spawning, %d\n"
msgstr "Error al lanzar el proceso %d\n"

#: job.c:1442
#, c-format
msgid "%s: Command not found"
msgstr "%s: No se encontró el programa"

# ¿Y "el programa Shell"? (a secas) sv
# Si. Suena mucho mejor. mm
# Suena, pero no es mejor. Tal y como esta escrito, incluso
# yo pensaria que me falta un programa en mi sistema que se
# llame shell. No se ha encontrado el 'shell'em+
# Es bueno contar con una segunda opinión ... mm
#: job.c:1471
#, c-format
msgid "%s: Shell program not found"
msgstr "%s: No se ha encontrado el `shell'"

# Lo mismo de arriba con "ignorar". sv
# Ok. mm.
#: main.c:224
msgid "Ignored for compatibility"
msgstr "No se tendrá en cuenta por compatibilidad"

#: main.c:227
msgid "Change to DIRECTORY before doing anything"
msgstr "Debe desplazarse al directorio DIRECTORY antes de hacer cualquier cosa"

# ¿"depurado" o "depuración"? sv
# Puse de depurado para evitar la cacofonía información depuración.
# Sugerencias bienvenidas. mm
#: main.c:230
msgid "Print lots of debugging information"
msgstr "Se imprimirán grandes cantidades de información de depurado"

#: main.c:233
msgid "Environment variables override makefiles"
msgstr "Las variables de entorno tienen prioridad sobre los makefiles"

#: main.c:236
msgid "Read FILE as a makefile"
msgstr "Se leyó el ARCHIVO como makefile"

#: main.c:239
msgid "Print this message and exit"
msgstr "Muestra este mensaje y finaliza"

# Ojo con ignorar. sv
# Ok. mm
# Por favor, si has traducido commands como instrucciones , hazlo aqui
# tambien.
# No me gusta este mensaje, preferiría: Se ignoran los errores obtenidos
# en la ejecución de las instrucciones  em+
#: main.c:242
msgid "Ignore errors from commands"
msgstr "No se toman en cuenta los errores provenientes de los comandos"

#: main.c:245
msgid "Search DIRECTORY for included makefiles"
msgstr "Se buscan en DIRECTORIO los archivos makefile incluídos"

# Yo traduciría "infinite" por "infinitos", no por "una infinidad", que
# parece que son muchos menos... sv
# Ok. mm
#: main.c:249
msgid "Allow N jobs at once; infinite jobs with no arg"
msgstr ""
"Se permiten N trabajos a la vez; si no se especifica un\n"
"argumento son infinitos"

# No entiendo por qué aquí empleas subjuntivo: "pudieron". sv
# Es incorrecto. Es una de las opciones `k' que dice... mm
#: main.c:253
msgid "Keep going when some targets can't be made"
msgstr ""
"Sigue avanzando aún cuando no se puedan crear algunos objetivos"

# Esto me suena a descripción de una opción, más que a un mensaje de error.
# Sugerencia por lo tanto: "No se lanzan ..." sv
# Estás en lo correcto. mm
#: main.c:258 main.c:263
msgid "Don't start multiple jobs unless load is below N"
msgstr ""
"No se lanzan varios trabajos a menos que la carga\n"
"sea inferior a N"

# Lo mismo de antes. sv
# Ok. mm
#: main.c:270
msgid "Don't actually run any commands; just print them"
msgstr "No se ejecutan las instrucciones; sólamente se muestran"

#: main.c:273
msgid "Consider FILE to be very old and don't remake it"
msgstr "Se considera el ARCHIVO demasiado viejo y no se reconstruye"

# de 'make' em+
# ok. mm
#: main.c:276
msgid "Print make's internal database"
msgstr "Se imprime la base de datos interna de `make'"

#: main.c:279
msgid "Run no commands; exit status says if up to date"
msgstr ""
"No se ejecutan las instrucciones; el estado de salida\n"
"indicará si están actualizados"

# ¿desabilitan o deshabilitan? sv
# Error de dedo. mm
# ¡ Por favor ! , ¿ pero qué es eso de interconstruidas ?
# Pon almacenadas internamente, que es exactamente lo que son :) em+
# Bueno, bajo protesta. mm
#: main.c:282
msgid "Disable the built-in implicit rules"
msgstr "Se deshabilitan las reglas implícitas almacenadas internamente"

# ¿ qué te parece ...
# No se hace echo de las instrucciones em+
# No me gusta. Que te parece lo que puse ahora? mm
#: main.c:285
msgid "Don't echo commands"
msgstr "Los comandos no se muestran con eco"

# Corregido error de tecleos ;) em+
# Ok.
#: main.c:289
msgid "Turns off -k"
msgstr "Se deshabilita -k"

# Se *tocan*, ¿no? sv
# Si. mm
#
# Pues entonces cámbialo arriba tambien em+
# En donde? mm
#: main.c:292
msgid "Touch targets instead of remaking them"
msgstr "Se tocan los objetivos en vez de reconstruirlos"

# Me comería el "Se" inicial. "Muestra la versión..." sv
# Ok. mm
#: main.c:295
msgid "Print the version number of make and exit"
msgstr "Muestra la versión del make y finaliza"

# Lo mismo. sv
#: main.c:298
msgid "Print the current directory"
msgstr "Muestra el directorio actual"

# Lo mismo. sv
# Turn off , desactiva o deshabilita , pero no apaga em+
# apaga luz Mari Luz apaga luz ,que yo no puedo vivir con
# tanta luz ... ( canción tradicional ) ( no lo pude evitar ) em+
# Juar, Juar, Juar. Coincido, pero el sentido es el mismo. mm
#: main.c:301
msgid "Turn off -w, even if it was turned on implicitly"
msgstr "Desactiva -w, aún cuando haya sido activado implícitamente"

# Lo mismo. sv
# Se considera siempre como nuevo em+
# Ok. Elimino el 'se' porque está describiendo a una opción. mm
#: main.c:304
msgid "Consider FILE to be infinitely new"
msgstr "Considera al ARCHIVO siempre como nuevo"

# Lo mismo. sv
# Todas estas parecen descripciones de opciones.
#: main.c:307
msgid "Warn when an undefined variable is referenced"
msgstr "Advierte cuando se hace una referencia a una variable no definida"

#: main.c:394
msgid "empty string invalid as file name"
msgstr "no se permite que una cadena vacía sea el nombre de un archivo"

#: main.c:781
msgid "fopen (temporary file)"
msgstr "fopen (archivo temporal)"

#: main.c:787
msgid "fwrite (temporary file)"
msgstr "fwrite (archivo temporal)"

# Antes pusiste "makefiles". Coherencia. sv
# Cierto. mm
#: main.c:930
msgid "Updating makefiles...."
msgstr "Actualizando archivos makefiles...."

# Me comería el "archivo" inicial.
# Ok. mm
# "El Makefile `%s' ..." sv
# Pondría ( como en un mensaje anterior ) se autoreferencia ... em+
# He puesto makefile con minúsculas , par ser coherentes em+
# Ok. Es más breve. mm.
#: main.c:955
#, c-format
msgid "Makefile `%s' might loop; not remaking it.\n"
msgstr ""
"El makefile `%s' se autoreferencia; por lo cual no se reconstruye.\n"

# Lo mismo. sv
#: main.c:1029
#, c-format
msgid "Failed to remake makefile `%s'."
msgstr "Fallo al reconstruir el makefile `%s'."

# Lo mismo. sv
#: main.c:1044
#, c-format
msgid "Included makefile `%s' was not found."
msgstr "No se encontró el makefile incluído `%s'."

# Lo mismo. sv
#: main.c:1049
#, c-format
msgid "Makefile `%s' was not found"
msgstr "No se encontró el Makefile `%s'"

#: main.c:1108
msgid "Couldn't change back to original directory."
msgstr "No se pudo regresar al directorio original."

#: main.c:1142
msgid "Re-executing:"
msgstr "Re-ejecutando:"

#: main.c:1186
msgid "Updating goal targets...."
msgstr "Actualizando los objetivos finales...."

# Sugerencia: "No se especificó ningún objetivo ... " sv
# Ok. Me gusta. mm
#: main.c:1211
msgid "No targets specified and no makefile found"
msgstr "No se especificó ningún objetivo y no se encontró ningún makefile"

#: main.c:1213
msgid "No targets"
msgstr "No hay objetivos"

# Me comería el "de" de "requiere de" sv
# Ok. mm
#: main.c:1439
#, c-format
msgid "the `-%c' option requires a positive integral argument"
msgstr "la opción `-%c' requiere un argumento positivo y entero"

# ¿Y Modo de empleo? sv
# Soy medio bestia. Debería hacerlo automático. mm
# "target" es "objetivo", no "objetivos". Fíjate que lleva puntos
# suspensivos, permitiendo así varios objetivos. sv
# Ok. mm
#: main.c:1490
#, c-format
msgid "Usage: %s [options] [target] ...\n"
msgstr "Modo de empleo: %s [opciones] [objetivo] ...\n"

#: main.c:1492
msgid "Options:\n"
msgstr "Opciones:\n"

# No me gusta como queda así.
# Sugerencia: "%sGNU Make versión %s" sv
# Aceptada. mm
#: main.c:1967
#, c-format
msgid "%sGNU Make version %s"
msgstr "%sGNU Make versión %s"

# De esto hablaremos otro día. sv
# Si. Es bastante complicado. mm
#: main.c:1971
#, c-format
msgid ""
", by Richard Stallman and Roland McGrath.\n"
"%sCopyright (C) 1988, 89, 90, 91, 92, 93, 94, 95 Free Software Foundation, "
"Inc.\n"
"%sThis is free software; see the source for copying conditions.\n"
"%sThere is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
"%sPARTICULAR PURPOSE.\n"
"\n"
msgstr ""
", por Richard Stallman y Roland McGrath.\n"
"%sCopyright (C) 1988, 89, 90, 91, 92, 93, 94, 95 Free Software Foundation, "
"Inc.\n"
"%sEste es software libre; consulte en el código fuente las condiciones de "
"copia.\n"
"%sNO hay garantía; ni siquiera para MERCANTIBILIDAD o EL CUMPLIMIENTO DE\n"
"%sALGÚN PROPÓSITO PARTICULAR.\n"
"\n"

#: main.c:1993
#, c-format
msgid ""
"\n"
"# Make data base, printed on %s"
msgstr ""
"\n"
"# Base de datos del Make, mostrada en %s"

# make -> Make. sv
# Ok. mm
# Porqué 'del', o pones 'del programa' Make o pones
# 'de Make' em+
# Ok. mm
#: main.c:2002
#, c-format
msgid ""
"\n"
"# Finished Make data base on %s\n"
msgstr ""
"\n"
"# Se termina la base de datos de Make en %s\n"

# "Entrando en el" (fíjate que esto lo dice cuando entra en un directorio).
# Habría que ver cómo queda. sv
# Ver nota más adelante. mm
# Cambiando a , me pareceria mucho mejor em+
# En efecto. mm
#: main.c:2053
msgid "Entering"
msgstr "Cambiando a"

# "Dejando el", "Abandonando el". sv
# Que tal "saliendo"? mm
# Prefiero 'abandonando' , o 'saliendo de' em+
# Que tal saliendo? mm
#: main.c:2053
msgid "Leaving"
msgstr "Saliendo"

#: main.c:2072
msgid "an unknown directory"
msgstr "un directorio desconocido"

# Probablemente sea aquí donde haya que ponerle el "el" para que "Entering"
# concuerde bien con esta frase y con la anterior simultáneamente. sv
# No. Ya revisé el fuente y transcribo el trozo pertinente:
#  char *message = entering ? "Entering" : "Leaving";
#  if (makelevel == 0)
#    printf ("%s: %s ", program, message);
#  else
#    printf ("%s[%u]: %s ", program, makelevel, message);
# Como notarás lo del directorio va en otro lado.
#: main.c:2074
#, c-format
msgid "directory `%s'\n"
msgstr "directorio `%s'\n"

# Interrumpido (?). sv
# Mmgmh... El mensaje indica que hubo algún error muy grave y que por
# eso se detiene el make. Probablemente sea mejor dejarlo así. mm
#: misc.c:212 misc.c:260
msgid ".  Stop.\n"
msgstr ".  Alto.\n"

#: misc.c:277
msgid "Unknown error 12345678901234567890"
msgstr "Error desconocido 12345678901234567890"

#: misc.c:282
#, c-format
msgid "Unknown error %d"
msgstr "Error desconocido %d"

# Propongo eliminar la palabra virtual. sv
# Pero si es muy bonita. :) Bueno. Como ya platicamos puede que sea
# una buena idea pero, por ahora la dejaría. mm
#: misc.c:318 misc.c:330 read.c:2151
msgid "virtual memory exhausted"
msgstr "memoria virtual agotada"

#: misc.c:536
#, c-format
msgid "%s access: user %d (real %d), group %d (real %d)\n"
msgstr "%s acceso: usuario %d (real %d), grupo %d (real %d)\n"

#: misc.c:556
msgid "Initialized"
msgstr "Inicializado"

#: misc.c:635
msgid "User"
msgstr "Usuario"

#: misc.c:683
msgid "Make"
msgstr "Make"

#: misc.c:717
msgid "Child"
msgstr "Hijo"

# Sugerencia: eliminar la palabra "archivo". sv
# Ok. Mejora. mm
#: read.c:129
msgid "Reading makefiles..."
msgstr "Leyendo makefiles..."

#: read.c:298
#, c-format
msgid "Reading makefile `%s'"
msgstr "Leyendo makefile `%s'"

#: read.c:300
msgid " (no default goal)"
msgstr " (no hay objetivo por defecto)"

#: read.c:302
msgid " (search path)"
msgstr " (ruta de búsqueda)"

#: read.c:304
msgid " (don't care)"
msgstr " (no importa)"

#: read.c:306
msgid " (no ~ expansion)"
msgstr " (no hay expansión del ~)"

# ## Sintaxis no llevaba tilde.
# Ok. mm
#: read.c:466
msgid "invalid syntax in conditional"
msgstr "sintaxis no válida en condicional"

#: read.c:474
msgid "extraneous `endef'"
msgstr "el `endef' es irrelevante o está mal colocado"

#: read.c:500 read.c:522
msgid "empty `override' directive"
msgstr "instrucción `override' vacía"

# corregido el efecto indio , añado un 'un'em+
# Ok. mm
#: read.c:584
#, c-format
msgid "no file name for `%sinclude'"
msgstr "no hay un nombre de archivo para `%sinclude'"

#: read.c:670
msgid "commands commence before first target"
msgstr "las instrucciones comenzaron antes del primer objetivo"

# "falta una regla". sv
# (es que extraviada me suena muy raro).
# Cierto pareciera que se perdió dentro de algún circuito. mm
# Falta 'la' regla em+
# No no es la regla sino una regla pues puede ser cualquiera de ellas. mm
#: read.c:714
msgid "missing rule before commands"
msgstr "falta una regla antes de las instrucciones"

#: read.c:733
msgid "missing separator"
msgstr "falta un separador"

#: read.c:782
msgid "missing target pattern"
msgstr "falta un patrón de objetivos"

#: read.c:784
msgid "multiple target patterns"
msgstr "hay varios patrones de objetivos"

#: read.c:789
msgid "target pattern contains no `%%'"
msgstr "el patrón de objetivo no contiene `%%'"

#: read.c:829
msgid "missing `endif'"
msgstr "falta un `endif'"

#: read.c:887
msgid "Extraneous text after `endef' directive"
msgstr ""
"Hay un texto irrelevante o mal colocado después de la instrucción `endef'"

#: read.c:917
msgid "missing `endef', unterminated `define'"
msgstr "falta un `endef', no se terminó un `define'"

#: read.c:973 read.c:1120
#, c-format
msgid "Extraneous text after `%s' directive"
msgstr "Texto irrelevante o mal colocado después de la instrucción `%s'"

#: read.c:977
#, c-format
msgid "extraneous `%s'"
msgstr "irrelevante o mal colocado `%s'"

#: read.c:982
msgid "only one `else' per conditional"
msgstr "sólo se admite un `else' por condicional"

#: read.c:1230
msgid "mixed implicit and static pattern rules"
msgstr "las reglas implícitas y las de patrón estático están mezcladas"

# Y 'las' normales  em+
# Ok. mm
#: read.c:1233
msgid "mixed implicit and normal rules"
msgstr "las reglas implícitas y las normales están mezcladas"

#: read.c:1273
#, c-format
msgid "target `%s' doesn't match the target pattern"
msgstr "el archivo de objetivos `%s' no coincide con el patrón de objetivos"

#: read.c:1305 read.c:1407
#, c-format
msgid "target file `%s' has both : and :: entries"
msgstr "el archivo de objetivos `%s' tiene líneas con : y ::"

# Cambiaría given por 'proporcionó' o 'indicó' em+
# Ok. mm
#: read.c:1313
#, c-format
msgid "target `%s' given more than once in the same rule."
msgstr "el objetivo `%s' se proporcionó más de una vez en la misma regla."

# No me gusta esta traducción de override. Mira a ver
# si encaja mejor alguna de las dos que se proponen arriba
# em+
# Aunque no me acaba de convencer, que te parece ésto? mm
#: read.c:1322
#, c-format
msgid "warning: overriding commands for target `%s'"
msgstr "atención: se imponen comandos para el objetivo `%s'"

# Ojo con ignora. sv
#: read.c:1325
#, c-format
msgid "warning: ignoring old commands for target `%s'"
msgstr "atención: se ignoran las instrucciones viejas para el objetivo `%s'"

#: read.c:1815
msgid "warning: NUL character seen; rest of line ignored"
msgstr "atención: hay un carácter NUL; se ignora el resto de la línea"

#: remake.c:212
#, c-format
msgid "Nothing to be done for `%s'."
msgstr "No se hace nada para `%s'."

#: remake.c:213
#, c-format
msgid "`%s' is up to date."
msgstr "`%s' está actualizado."

# ¿target file no sería "archivo objetivo"? sv
# Literalmente si pero un archivo make puede tener varios objetivos. mm
# Max , pero no existe un archivo de objetivos. Esta línea, que es
# de las que aparecen al hacer un make con la opción -d ( debug )
# se refiere a lo que dice Santiago, es decir, 'archivo objetivo'  em+
# Ok creo que tienen razón. mm
#: remake.c:310
#, c-format
msgid "Considering target file `%s'.\n"
msgstr "Se considera el archivo objetivo `%s'.\n"

# Un compañero mío dice que una buena regla es poner siempre que se pueda
# un número par de comas.
# Es decir: "Se intentó, sin éxito, actualizar ..."
# o bien "Se intentó sin éxito actualizar ..."
# ¿Qué te parece? sv
# Bien y tiene razón. mm
#: remake.c:316
#, c-format
msgid "Recently tried and failed to update file `%s'.\n"
msgstr "Se intentó, sin éxito, actualizar el archivo `%s'.\n"

#: remake.c:320
#, c-format
msgid "File `%s' was considered already.\n"
msgstr "El archivo `%s' ya fue considerado.\n"

#: remake.c:330
#, c-format
msgid "Still updating file `%s'.\n"
msgstr "Se continúa actualizando el archivo `%s'.\n"

#: remake.c:333
#, c-format
msgid "Finished updating file `%s'.\n"
msgstr "Se terminó de actualizar el archivo `%s'.\n"

#: remake.c:354
#, c-format
msgid "File `%s' does not exist.\n"
msgstr "El archivo `%s' no existe.\n"

#: remake.c:364 remake.c:728
#, c-format
msgid "Found an implicit rule for `%s'.\n"
msgstr "Se encontró una regla implícita para `%s'.\n"

# Se ha encontrado em+
# Ok, pero procuro evitar como a la muerte los gerundios. mm
#: remake.c:366 remake.c:730
#, c-format
msgid "No implicit rule found for `%s'.\n"
msgstr "No se ha encontrado una regla implícita para `%s'.\n"

# Por defecto, como haces arriba em+
# en efecto, mm
#: remake.c:372 remake.c:736
#, c-format
msgid "Using default commands for `%s'.\n"
msgstr "Se utilizan las instrucciones por defecto para `%s'.\n"

#: remake.c:392 remake.c:760
#, c-format
msgid "Circular %s <- %s dependency dropped."
msgstr "Se elimina la dependencia circular %s <- %s."

# Lo mismo de antes con "target file". sv
# Estoy de acuerdo con él ( por una vez ;) ) em+
# Ok. mm
#: remake.c:474
#, c-format
msgid "Finished dependencies of target file `%s'.\n"
msgstr "Se terminaron las dependencias del archivo objetivo `%s'.\n"

#: remake.c:480
#, c-format
msgid "The dependencies of `%s' are being made.\n"
msgstr "Se están construyendo las dependencias de `%s'.\n"

# Give up no es enfocar, es abandonar, o desistir em+
# Si, metí la pata. mm
#: remake.c:493
#, c-format
msgid "Giving up on target file `%s'.\n"
msgstr "Se abandona el archivo objetivo `%s'.\n"

#: remake.c:497
#, c-format
msgid "Target `%s' not remade because of errors."
msgstr "Debido a los errores, el objetivo `%s' no se reconstruyó."

#: remake.c:542
#, c-format
msgid "Dependency `%s' does not exist.\n"
msgstr "La dependencia `%s' no existe.\n"

#: remake.c:544
#, c-format
msgid "Dependency `%s' is %s than dependent `%s'.\n"
msgstr "La dependencia `%s' es %s que el dependiente `%s'.\n"

#: remake.c:545
msgid "newer"
msgstr "más reciente"

#: remake.c:545
msgid "older"
msgstr "más antigua"

# Ahorra espacio, y pon '::'  en vez de eso :) em+
# Bueno, no es mala idea. A ver si te gusta lo que puse. mm
#: remake.c:556
#, c-format
msgid "Target `%s' is double-colon and has no dependencies.\n"
msgstr ""
"El objetivo `%s' es de tipo dos puntos dos veces (::)\n"
"y no tiene dependencias.\n"

#: remake.c:561
#, c-format
msgid "No commands for `%s' and no dependencies actually changed.\n"
msgstr "No hay instrucciones para `%s' y ninguna dependencia cambió.\n"

#: remake.c:566
#, c-format
msgid "No need to remake target `%s'.\n"
msgstr "No es necesario reconstruir el objetivo `%s'.\n"

# Revisa todo el po con un search, y mira a ver si decides usar
# regenerar o reconstruir ( prefiero lo último 10000 veces ) em+
# Ok, buena propuesta. mm
#: remake.c:571
#, c-format
msgid "Must remake target `%s'.\n"
msgstr "Se debe reconstruir el objetivo `%s'.\n"

#: remake.c:578
#, c-format
msgid "Commands of `%s' are being run.\n"
msgstr "Las instrucciones de `%s' se están ejecutando.\n"

# Target file no es archivo de objetivos, sino el archivo objetivo
# make no tiene ningún archivo de objetivos em+
# Ok. mm
#: remake.c:585
#, c-format
msgid "Failed to remake target file `%s'.\n"
msgstr "Fallo al reconstruir el archivo objetivo `%s'.\n"

# Lo mismo em+
# Ok. mm
#: remake.c:588
#, c-format
msgid "Successfully remade target file `%s'.\n"
msgstr "Se reconstruyó con éxito el archivo objetivo `%s'.\n"

# Otra vez em+
# Ok, no te puedes quejar de falta de consistencia en este caso! mm
#: remake.c:591
#, c-format
msgid "Target file `%s' needs remade under -q.\n"
msgstr ""
"Se necesita reconstruir el archivo objetivo `%s' con la opción -q.\n"

# Sugerencia: No hay ninguna regla... sv
# Como una no hay ninguna. mm
#: remake.c:880
#, c-format
msgid "%sNo rule to make target `%s'%s"
msgstr "%sNo hay ninguna regla para construir el objetivo `%s'%s"

#: remake.c:882
#, c-format
msgid "%sNo rule to make target `%s', needed by `%s'%s"
msgstr ""
"%sNo hay ninguna regla para construir el objetivo `%s', necesario para `%s'%s"

# Ubicada :)) , que tal futura, a secas ? em+
# Bueno, sonaba como StarTrek, muy chido, pero acepto tu sugerencia. mm
#: remake.c:1053
#, c-format
msgid "*** File `%s' has modification time in the future"
msgstr ""
"*** El archivo `%s' tiene una fecha de modificación futura"

# ¿Las aduanas? sv
# Si, las aduanas de exportación de procesos. Suena rarísimo pero
# revisé el código y todo parece apuntar a que así son las cosas.
# Creo que esto es otra cosa. Custom se refiere a los valores dados
# por el usuario ( customizables ) , y que no se exportarán se refiere
# a que al llamar a otros makefiles, no serán pasados como valores
# que se antepongan ( otro término para override ) a los que make
# tiene por defecto .
# La traducción exacta por tanto es, los valores definidos por el usuario
# no se exportarán em+
# Muchísimo más claro (es más me gusta más en español que en inglés con tu
# arreglo) mm
#: remote-cstms.c:94
#, c-format
msgid "Customs won't export: %s\n"
msgstr "Los valores definidos por el usuario no se exportarán: %s\n"

# Sugerencia: `socket' sv
# Ok. mm
#: remote-cstms.c:129
msgid "exporting: Couldn't create return socket."
msgstr "exportando: No se puede crear el `socket' de regreso."

#: remote-cstms.c:138
msgid "exporting: "
msgstr "exportando: "

#: remote-cstms.c:171
#, c-format
msgid "exporting: %s"
msgstr "exportando: %s"

#: remote-cstms.c:185
#, c-format
msgid "Job exported to %s ID %u\n"
msgstr "El trabajo ha sido exportado a %s ID %u\n"

#: rule.c:556
msgid ""
"\n"
"# Implicit Rules"
msgstr ""
"\n"
"# Reglas implícitas."

#: rule.c:571
msgid ""
"\n"
"# No implicit rules."
msgstr ""
"\n"
"# No hay reglas implícitas."

#: rule.c:574
#, c-format
msgid ""
"\n"
"# %u implicit rules, %u"
msgstr ""
"\n"
"# %u reglas implícitas, %u"

#: rule.c:583
msgid " terminal."
msgstr " terminal."

# ## Añado ¡ con tu permiso. sv
# Gracias. mm
# ¿ qué tal erróneo ? em+
# Si, suena mejor. mm
#: rule.c:587
#, c-format
msgid "BUG: num_pattern_rules wrong!  %u != %u"
msgstr "BUG: ¡num_pattern_rules erróneo!  %u != %u"

#: variable.c:658 variable.c:660
msgid "empty variable name"
msgstr "nombre de variable vacío"

# Sugerencia: No poner archivo. sv
# Ok. mm
#: variable.c:742
msgid "makefile"
msgstr "makefile"

# 'bajo -e' = 'con -e activo' em+
# Ok. Esta inversión en los idiomas sajones se pega. mm
#: variable.c:745
msgid "environment under -e"
msgstr "con -e activo"

# Creo que tendremos un problema con instrucción, comando
# , programa y directiva . Está claro que aquí es directiva em+
# Ok, pero no deja de sonar a RoboCop. A ver que te parece como quedó. mm
#: variable.c:751
msgid "`override' directive"
msgstr "directiva de sobreposición `override'"

#: variable.c:822
msgid "# No variables."
msgstr "# No hay variables."

# Bueno. Aquí un punto de discusión. Traduzco buckets por cubetas o
# mejor las dejo tal cual? Opiniones bienvenidas. mm
# Estos mensajes son para debug, no creo que haga falta usar cubetas ;) em+
# Tienes razón, si el debugueador no lo entiende pues, ... que se
# dedique a otra cosa. mm
#: variable.c:825
#, c-format
msgid "# %u variables in %u hash buckets.\n"
msgstr "# hay %u variables en %u `hash buckets´.\n"

#: variable.c:828
#, c-format
msgid "# average of %.1f variables per bucket, max %u in one bucket.\n"
msgstr "# promedio de %.1f variables por `bucket', máx %u en un `bucket'.\n"

#: variable.c:835
#, c-format
msgid "# average of %d.%d variables per bucket, max %u in one bucket.\n"
msgstr "# promedio de %d.%d variables por `bucket', máx %u en un `bucket'.\n"

#: variable.c:850
msgid ""
"\n"
"# Variables\n"
msgstr ""
"\n"
"# Variables\n"

# ¿Y al revés?: Rutas de búsqueda VPATH. sv
# Mejora
#: vpath.c:455
msgid ""
"\n"
"# VPATH Search Paths\n"
msgstr ""
"\n"
"# Rutas de búsqueda VPATH\n"

# Rutas creo que queda mejor. sv
# Ok. Es más común. mm
#: vpath.c:472
msgid "# No `vpath' search paths."
msgstr "# No hay rutas de búsqueda `vpath'."

# ¡Ah! Aquí si que pones el vpath al final, ¿eh? :-) sv
# You really got me! mm
#: vpath.c:474
#, c-format
msgid ""
"\n"
"# %u `vpath' search paths.\n"
msgstr ""
"\n"
"# %u rutas de búsqueda `vpath'.\n"

#: vpath.c:477
msgid ""
"\n"
"# No general (`VPATH' variable) search path."
msgstr ""
"\n"
"# No hay ruta de búsqueda general (variable `VPATH')."

#: vpath.c:483
msgid ""
"\n"
"# General (`VPATH' variable) search path:\n"
"# "
msgstr ""
"\n"
"# Ruta de búsqueda general (variable `VPATH'):\n"
"# "

#: getloadavg.c:948
msgid "Error getting load average"
msgstr "Error al obtener la carga promedio"

#: getloadavg.c:952
#, c-format
msgid "1-minute: %f  "
msgstr "1-minuto: %f  "

#: getloadavg.c:954
#, c-format
msgid "5-minute: %f  "
msgstr "5-minutos: %f  "

#: getloadavg.c:956
#, c-format
msgid "15-minute: %f  "
msgstr "15-minutos: %f  "

#: getopt.c:565
#, c-format
msgid "%s: option `%s' is ambiguous\n"
msgstr "%s: la opción `%s' es ambigua\n"

# No admite ningún argumento. sv
# Ok. Y vuelve la burra al trigo. mm
#: getopt.c:589
#, c-format
msgid "%s: option `--%s' doesn't allow an argument\n"
msgstr "%s: la opción `--%s' no admite ningún argumento\n"

# Lo mismo. sv
# Ok. mm
#: getopt.c:594
#, c-format
msgid "%s: option `%c%s' doesn't allow an argument\n"
msgstr "%s: la opción `%c%s' no admite ningún argumento\n"

#: getopt.c:611
#, c-format
msgid "%s: option `%s' requires an argument\n"
msgstr "%s: la opción `%s' requiere un argumento\n"

#: getopt.c:640
#, c-format
msgid "%s: unrecognized option `--%s'\n"
msgstr "%s: opción no reconocida `--%s'\n"

#: getopt.c:644
#, c-format
msgid "%s: unrecognized option `%c%s'\n"
msgstr "%s: opción no reconocida `%c%s'\n"

#: getopt.c:670
#, c-format
msgid "%s: illegal option -- %c\n"
msgstr "%s: opción inválida -- %c\n"

#: getopt.c:673
#, c-format
msgid "%s: invalid option -- %c\n"
msgstr "%s: opción inválida -- %c\n"

#: getopt.c:709
#, c-format
msgid "%s: option requires an argument -- %c\n"
msgstr "%s: la opción requiere un argumento -- %c\n"

# argv-elements -> elementos argv. sv
# (A poco C que sepas, sabrás lo que es).
# Algunos si, algunos no. Pero tienes razón. mm
#: getopt.c:777 getopt1.c:141
msgid "digits occur in two different argv-elements.\n"
msgstr "los dígitos están en dos elementos argv distintos.\n"

#: getopt.c:779 getopt1.c:143
#, c-format
msgid "option %c\n"
msgstr "opción %c\n"

#: getopt.c:783 getopt1.c:147
msgid "option a\n"
msgstr "opción a\n"

#: getopt.c:787 getopt1.c:151
msgid "option b\n"
msgstr "opción b\n"

# Sugerencia: La opción c tiene el valor `%s'
# Hay un mensaje idéntico en hello. sv
# Me gusta mucho. mm
#: getopt.c:791 getopt1.c:155
#, c-format
msgid "option c with value `%s'\n"
msgstr "La opción c tiene el valor `%s'\n"

#: getopt.c:798 getopt1.c:166
#, c-format
msgid "?? getopt returned character code 0%o ??\n"
msgstr "?? getopt() regresó un cáracter con código 0%o ??\n"

# Elementos ARGV que no son opciones: sv
# Ok. mm.
# Esto es:
# Elementos de ARGV 'que' no son opciones:
# pero tambien es un mensaje de debug del propio make, asi
# que no me preocupa ( no aparecerá a usuarios normales de make ) em+
# Bueno, pero de todas formas se corrige. mm
#: getopt.c:804 getopt1.c:172
msgid "non-option ARGV-elements: "
msgstr "los elementos ARGV que no son opciones: "

#: getopt1.c:159
#, c-format
msgid "option d with value `%s'\n"
msgstr "opción d con valor `%s'\n"

#: signame.c:57
msgid "unknown signal"
msgstr "señal desconocida"

#: signame.c:107
msgid "Hangup"
msgstr "Colgado"

# ¿ Y por qué no Interrupción ? em+
# Porque es una acción. De hecho es un mensaje que se envía a través
# del sistema en este caso le enviarías un kill -INT num_proceso para
# interrumpir al programa. Lo revisé contra el fuente. mm
#: signame.c:110
msgid "Interrupt"
msgstr "Interrumpir"

#: signame.c:113
msgid "Quit"
msgstr "Finalizar"

#: signame.c:116
msgid "Illegal Instruction"
msgstr "Instrucción no válida"

# Los trap los hemos dejado como traps, simplemente. em+
# Ok. mm
#: signame.c:119
msgid "Trace/breakpoint trap"
msgstr "Trace/breakpoint trap"

#: signame.c:124
msgid "Aborted"
msgstr "Abortado"

# Ponte de acuerdo con Enrique en cómo se traduce esto.
# Enrique tiene esta misma frase en glibc. sv
# Eso es :) IOT trap em+
# Ok. mm
#: signame.c:127
msgid "IOT trap"
msgstr "IOT trap"

# Otra em+
# Ok. mm
#: signame.c:130
msgid "EMT trap"
msgstr "EMT trap"

# Coma flotante, por favor. sv
# Creeme que aquí lo de coma flotante no vale. Somos pro-yanquis y
# por eso usamos el punto flotante. Debemos llegar a un acuerdo.
# Por ahora pongo coma flotante para facilitar las cosas, pero bajo
# protesta ;-) mm
#: signame.c:133
msgid "Floating point exception"
msgstr "Excepción de coma flotante"

# "Terminado por la señal kill" quedaría un poco más suave ... sv
# Enrique tiene en glibc "Terminado (killed)". sv
# Bueno, creo que mi traducción es más exacta, concisa y clara
# pero si insisten... Además recuerda en que estoy en un país
# en donde asesinado es palabra de todos los días. mm
#: signame.c:136
msgid "Killed"
msgstr "Terminado (killed)"

#: signame.c:139
msgid "Bus error"
msgstr "Error en el bus"

#: signame.c:142
msgid "Segmentation fault"
msgstr "Fallo de segmentación"

#: signame.c:145
msgid "Bad system call"
msgstr "Llamada al sistema errónea"

#: signame.c:148
msgid "Broken pipe"
msgstr "Tubería rota"

# ¿Temporizador? (así lo tradujo Enrique en glibc). sv
# Suena como StarTrek. Mejor lo platico con él. A lo mejor
# hasta lo convenzo. mm
# Pues tendrás que darme razones em+
# Bueno, un alarm clock es una alarma del reloj. No tiene pierde.
# Temporizador es una bonita palabra pero en donde dice que va a
# sonar una campana para despertarte? mm
#: signame.c:151
msgid "Alarm clock"
msgstr "Alarma del reloj"

#: signame.c:154
msgid "Terminated"
msgstr "Finalizado"

#: signame.c:157
msgid "User defined signal 1"
msgstr "Señal 1 definida por el usuario"

#: signame.c:160
msgid "User defined signal 2"
msgstr "Señal 2 definida por el usuario"

# Proceso hijo terminado em+
# Ok. mm
#: signame.c:165 signame.c:168
msgid "Child exited"
msgstr "Proceso hijo terminado"

# Fallo. sv
# Alimentación eléctrico em+
# Ok. mm
#: signame.c:171
msgid "Power failure"
msgstr "Falla de alimentación eléctrica"

#: signame.c:174
msgid "Stopped"
msgstr "Detenido"

# Enrique hizo una preciosa traducción de este mensaje que, según él,
# mejora el original. Era algo así como:
# "Detenido (requiere entrada de terminal)". sv
# Tiene razón se ve bien. mm
# Requiere 'de';) , como te vea Santiago que le metes
# otro 'de' otra vez ... em+
# Ok. mm
#: signame.c:177
msgid "Stopped (tty input)"
msgstr "Detenido (se requiere entrada de terminal)"

# lo mismo, se requiere entrada de terminal ,  em+
# Ok. mm
#: signame.c:180
msgid "Stopped (tty output)"
msgstr "Detenido (se requiere salida de terminal)"

# idem em+
#: signame.c:183
msgid "Stopped (signal)"
msgstr "Detenido (se requiere una señal)"

#: signame.c:186
msgid "CPU time limit exceeded"
msgstr "Se agotó el tiempo de CPU permitido"

#: signame.c:189
msgid "File size limit exceeded"
msgstr "Se excedió el tamaño máximo de archivo permitido"

#: signame.c:192
msgid "Virtual timer expired"
msgstr "El contador de tiempo virtual ha expirado"

# Nunca me enteré de cómo se traducía profile, pero perfil me suena raro.
# ¿De dónde lo has sacado? sv
# Es lo que quiere decir, ni modo. Perfil de un avión es plane profile.
# Por cierto, lo más probable es que esta "traducción" haya que
# modificarla para que sea entendible. mm
# Esto lo tengo en glibc, lo mirare otro día. Estos mensajes no son
# importantes em+
# Agregué unas palabras en aras de claridad (espero) mm
#: signame.c:195
msgid "Profiling timer expired"
msgstr "El contador de tiempo para la generación del perfil ha expirado"

#: signame.c:201
msgid "Window changed"
msgstr "La ventana ha cambiado"

#: signame.c:204
msgid "Continued"
msgstr "Continuado"

#: signame.c:207
msgid "Urgent I/O condition"
msgstr "Condición urgente de I/O"

#: signame.c:214 signame.c:223
msgid "I/O possible"
msgstr "Posible I/O"

#: signame.c:217
msgid "SIGWIND"
msgstr "SIGWIND"

#: signame.c:220
msgid "SIGPHONE"
msgstr "SIGPHONE"

#: signame.c:226
msgid "Resource lost"
msgstr "Recurso perdido"

#: signame.c:229
msgid "Danger signal"
msgstr "Señal de peligro"

#: signame.c:232
msgid "Information request"
msgstr "Petición de información"

#: signame.c:286
#, c-format
msgid "%s: unknown signal"
msgstr "%s: señal desconocida"

#: signame.c:299
msgid "Signal 12345678901234567890"
msgstr "Señal 12345678901234567890"

#: signame.c:304
#, c-format
msgid "Signal %d"
msgstr "Señal %d"
