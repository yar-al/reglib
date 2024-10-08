
# reglib - C++ regex library
## Операторы

- метасимволы экранируются символом ‘%’

- Операция ‘или’: **r1|r2** (метасимвол ‘|’)

- Операция ‘конкатенация’: **r1r2**

 - Операция ‘замыкание Клини’: **r*** (метасимвол ‘*’)

 - Операция ‘позитивное замыкание’: **r+** (метасимвол ‘+’)

- Операция ‘символ из набора в диапазоне’: **[a1-an…]** (метасимвол ‘[ ]’)

- операторные скобки **(!r)**, определяющие приоритет операторов. (метасимвол ‘(! )’)

## Операции

- **findall** – поиск всех непересекающихся вхождений подстрок в строку соответствующих регулярному выражению

- **match** - проверка строки на соответствие регулярному выражению

- **compile** - РВ->ДКА
- **minimize** - ДКА->минимальный ДКА
- **get_re** - операция восстановления регулярного выражения методом исключения состояний.

- **inverse** - операция построения инверсии языка, задаваемым регулярным выражением (скомпилированным ДКА).

- **intersection** - Операция построения пересечения языков, задаваемых регулярными выражениями (скомпилированными ДКА).