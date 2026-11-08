# KNU FCC LaTeX Template

Шаблон для курсових, кваліфікаційних та методичних робіт ФКНК КНУ на базі `extarticle` і стилю `knu-fcc-style.sty`.

Стиль налаштовує українську мову, шрифти, поля сторінки, нумерацію, заголовки, зміст, підписи рисунків і таблиць, списки, титульну сторінку та допоміжні команди для типових структурних елементів роботи.

## Вміст проєкту

- `knu-fcc-style.sty` — основний стиль оформлення.
- `knu_methodical_elements_showcase.tex` — великий демонстраційний документ з прикладами більшості елементів.
- `main.tex` — мінімальний тестовий документ для перевірки XeLaTeX.
- `.vscode/settings.json` — локальна конфігурація VS Code LaTeX Workshop.
- `.gitignore` — виключення тимчасових LaTeX-файлів без виключення PDF.

## Швидкий старт

Створіть `.tex` файл або скопіюйте `knu_methodical_elements_showcase.tex`, після чого підключіть стиль:

```tex
\documentclass[14pt]{extarticle}
\usepackage{knu-fcc-style}
```

Рекомендовано збирати документ через XeLaTeX:

```powershell
latexmk -xelatex -synctex=1 -interaction=nonstopmode -file-line-error main.tex
```

Для повного оновлення змісту, посилань, кількості сторінок і статистики може знадобитися 2-3 проходи компіляції. `latexmk` виконує їх автоматично.

## VS Code + LaTeX Workshop

У проєкті вже додано `.vscode/settings.json` з recipe `latexmk (xelatex)`.

1. Відкрийте папку проєкту у VS Code.
2. Відкрийте потрібний `.tex` файл, наприклад `main.tex` або `knu_methodical_elements_showcase.tex`.
3. Запустіть збірку через `Ctrl+Alt+B`.
4. Відкрийте PDF через `Ctrl+Alt+V`.

Для ручної перевірки інструментів у терміналі:

```powershell
pdflatex --version
xelatex --version
latexmk -version
perl -v
```

## Рекомендована структура документа

Типова структура роботи може виглядати так:

```tex
\documentclass[14pt]{extarticle}
\usepackage{knu-fcc-style}

\begin{document}

\KNUTitlePageMake[
  type={Курсова робота},
  specialty={122 Комп'ютерні науки},
  title={Назва роботи},
  department={Назва кафедри},
  studentInfo={Виконав студент 2-го курсу},
  student={Прізвище Ім'я По батькові},
  supervisorInfo={Науковий керівник:\\доцент, к. ф.-м. н.},
  supervisor={Прізвище Ім'я По батькові},
  year={2026}
]

\KNUUnnumberedSection{Реферат}
\KNUWorkStats

\KNUKeywords{АЛГОРИТМ, МОДЕЛЬ, СИСТЕМА}

\KNUContents

\KNUUnnumberedSection{Вступ}
Текст вступу.

\section{Основний розділ}
Текст розділу.

\KNUUnnumberedSection{Висновки}
Текст висновків.

\begin{thebibliography}{9}
\bibitem{source1} Приклад джерела.
\end{thebibliography}

\appendix
\section{Додаток А}
Матеріали додатка.

\end{document}
```

## Що автоматично налаштовує стиль

### Мова і шрифти

Для XeLaTeX і LuaLaTeX стиль використовує `fontspec` і `polyglossia`, встановлює українську мову та намагається використати `Times New Roman`.

Якщо `Times New Roman` недоступний, стиль шукає fallback-шрифти:

- `Tinos`;
- `TeX Gyre Termes X`;
- `Latin Modern Roman`.

Для найкращої сумісності з кирилицею використовуйте XeLaTeX.

### Сторінка і абзаци

Стиль задає:

- формат A4;
- поля: верхнє 20 мм, нижнє 20 мм, ліве 25 мм, праве 10 мм;
- міжрядковий інтервал 1.5;
- абзацний відступ 1.27 см;
- нумерацію сторінок угорі праворуч.

Щоб приховати номер на поточній сторінці:

```tex
\KNUHidePageNumber
```

Титульна сторінка, створена через `\KNUTitlePageMake`, вже автоматично приховує номер.

### Заголовки

`\section` починається з нової сторінки, центрується, друкується жирним і переводиться у верхній регістр.

`\subsection`, `\subsubsection`, `\paragraph` і `\subparagraph` оформлюються жирним шрифтом з абзацного відступу.

Звичайні нумеровані розділи:

```tex
\section{Назва розділу}
\subsection{Назва підрозділу}
\subsubsection{Назва підпункту}
```

Ненумеровані структурні частини, які треба додати до змісту:

```tex
\KNUUnnumberedSection{Вступ}
\KNUUnnumberedSection{Висновки}
```

## Команди стилю

### `\KNUTitlePageMake`

Створює титульну сторінку. Команда приймає набір ключів:

```tex
\KNUTitlePageMake[
  key={value},
  key={value}
]
```

Основні ключі:

| Ключ | Призначення |
| --- | --- |
| `ministry` | Назва міністерства або вищої установи. |
| `university` | Назва університету. За замовчуванням КНУ імені Тараса Шевченка. |
| `faculty` | Назва факультету. За замовчуванням ФКНК. |
| `department` | Назва кафедри. |
| `type` | Тип роботи: курсова, кваліфікаційна, звіт тощо. |
| `degree` | Ступінь, наприклад `бакалавра` або `магістра`. |
| `specialty` | Спеціальність, наприклад `122 Комп'ютерні науки`. |
| `program` | Освітня програма. |
| `titleLabel` | Підпис перед темою. За замовчуванням `на тему:`. |
| `title` | Тема роботи. |
| `studentInfo` | Текст перед ПІБ студента. Може містити `\\` для перенесень. |
| `student` | ПІБ студента. |
| `studentSignature` | `true` або `false`; показувати місце для підпису студента. |
| `supervisorInfo` | Текст перед ПІБ керівника. |
| `supervisor` | ПІБ керівника. |
| `supervisorSignature` | `true` або `false`; показувати місце для підпису керівника. |
| `declaration` | Текст заяви про самостійність роботи. |
| `declarationSigner` | Підписант заяви, за замовчуванням `Студент`. |
| `admission` | Текст допуску до захисту. |
| `headOfDepartment` | Завідувач кафедри або відповідальна особа. |
| `city` | Місто, за замовчуванням `Київ`. |
| `year` | Рік, за замовчуванням поточний рік. |

Приклад розширеної титульної сторінки:

```tex
\KNUTitlePageMake[
  type={Кваліфікаційна робота},
  degree={бакалавра},
  specialty={122 Комп'ютерні науки},
  program={Комп'ютерні науки},
  title={Методи аналізу даних у прикладних задачах},
  department={Кафедра теоретичної кібернетики},
  studentInfo={Виконав студент 4-го курсу\\ОПП «Комп'ютерні науки»},
  student={Іваненко Іван Іванович},
  supervisorInfo={Науковий керівник:\\доцент, к. ф.-м. н.},
  supervisor={Петренко Петро Петрович},
  declaration={Засвідчую, що в цій роботі\\немає запозичень без посилань.},
  year={2026}
]
```

Щоб прибрати лінію підпису біля студента або керівника:

```tex
studentSignature={false}
supervisorSignature={false}
```

### `\KNUContents`

Виводить зміст з нової сторінки:

```tex
\KNUContents
```

Назва змісту автоматично задається як `ЗМІСТ`.

### `\KNUUnnumberedSection`

Створює ненумерований структурний розділ і додає його до змісту:

```tex
\KNUUnnumberedSection{Реферат}
\KNUUnnumberedSection{Вступ}
\KNUUnnumberedSection{Висновки}
```

Використовуйте цю команду для частин, які не повинні мати номера, але мають бути у змісті.

### `\KNUKeywords`

Друкує рядок ключових слів:

```tex
\KNUKeywords{АЛГОРИТМ, МОДЕЛЬ, ПРОГРАМНА СИСТЕМА}
```

### `KNUExample`

Оточення для оформлення прикладу:

```tex
\begin{KNUExample}
Нехай потрібно показати фрагмент методу, обчислення або пояснення.
\end{KNUExample}
```

### `\KNUWorkStats`

Виводить статистику для реферату:

```tex
\KNUWorkStats
```

Формат виводу:

```text
N с., N рис., N табл., N дод., N джерела.
```

Підрахунок працює так:

- сторінки беруться через мітку `LastPage`;
- рисунки рахуються за середовищем `figure`;
- таблиці рахуються за середовищем `table`;
- додатки рахуються як `\section` після `\appendix`;
- джерела рахуються за `\bibitem`.

Через це для коректного результату потрібно збирати документ кілька разів або використовувати `latexmk`.

Якщо замість `thebibliography` використовується `biblatex`, автоматичний підрахунок джерел треба адаптувати у `.sty`.

## Рисунки, таблиці, формули

### Рисунки

Рисунки нумеруються в межах розділу: `1.1`, `1.2`, `2.1`.

```tex
\begin{figure}[htbp]
  \centering
  \includegraphics[width=0.8\textwidth]{image.png}
  \caption{Назва рисунка}
  \label{fig:example}
\end{figure}
```

Посилання в тексті:

```tex
див. рисунок~\ref{fig:example}
```

### Таблиці

Таблиці також нумеруються в межах розділу. Стиль примусово розміщує таблиці через `[H]`, щоб вони не спливали далеко від місця вставки.

```tex
\begin{table}
  \caption{Назва таблиці}
  \label{tab:example}
  \centering
  \begin{tabular}{|l|c|}
    \hline
    Показник & Значення \\
    \hline
    Точність & 95\% \\
    \hline
  \end{tabular}
\end{table}
```

Посилання:

```tex
див. таблицю~\ref{tab:example}
```

### Формули

Формули нумеруються в межах розділу:

```tex
\begin{equation}
  \label{eq:sum}
  S = \sum_{i=1}^{n} i = \frac{n(n+1)}{2}.
\end{equation}
```

Посилання:

```tex
див. формулу~\eqref{eq:sum}
```

## Списки

Маркеровані списки першого і другого рівня використовують тире:

```tex
\begin{itemize}
  \item перший пункт;
  \item другий пункт.
\end{itemize}
```

Нумеровані списки:

```tex
\begin{enumerate}
  \item перший пункт;
  \item другий пункт.
\end{enumerate}
```

Стиль робить списки компактними: без зайвих вертикальних відступів між пунктами.

## Посилання і URL

Стиль підключає `hyperref` з прихованим оформленням посилань (`hidelinks`) і `url`.

```tex
Документація доступна за адресою \url{https://example.com}.
```

Внутрішні посилання робіть через `\label`, `\ref`, `\pageref` та `\eqref`.

## Бібліографія

Базовий варіант через `thebibliography`:

```tex
\begin{thebibliography}{9}
\bibitem{knuth} Knuth D. The TeXbook. Addison-Wesley, 1984.
\bibitem{lamport} Lamport L. LaTeX: A Document Preparation System. Addison-Wesley, 1994.
\end{thebibliography}
```

Назва розділу бібліографії автоматично задається як `ПЕРЕЛІК ДЖЕРЕЛ ПОСИЛАННЯ`.

Посилання в тексті:

```tex
Цей підхід описано в~\cite{knuth}.
```

## Додатки

Після основної частини використовуйте стандартну команду:

```tex
\appendix

\section{Додаток А}
Текст додатка.

\section{Додаток Б}
Текст другого додатка.
```

Кожен `\section` після `\appendix` враховується у `\KNUWorkStats` як окремий додаток.

## Типові проблеми

### Не знаходиться `latexmk`, `xelatex` або `perl`

Перевірте команди:

```powershell
pdflatex --version
xelatex --version
latexmk -version
perl -v
```

Якщо команда не знаходиться, додайте до системного `PATH` шляхи MiKTeX і Strawberry Perl, наприклад:

```text
C:\Program Files\MiKTeX\miktex\bin\x64
C:\Strawberry\perl\bin
C:\Strawberry\c\bin
```

Після зміни `PATH` перезапустіть VS Code.

### Не оновлюється зміст або статистика

Запустіть збірку ще раз або використовуйте `latexmk`. У VS Code достатньо `Ctrl+Alt+B`, бо recipe вже використовує `latexmk`.

### Немає Times New Roman

Для Windows це рідко, але якщо шрифт недоступний, стиль використає fallback. Якщо потрібна сувора відповідність, встановіть `Times New Roman` або перевірте доступність шрифту в системі.

### Український текст відображається некоректно в PowerShell

Файли зберігаються у UTF-8. Якщо старий Windows PowerShell показує mojibake, відкривайте файл у VS Code або читайте з явним кодуванням:

```powershell
Get-Content -Encoding UTF8 main.tex
```

## Очищення тимчасових файлів

LaTeX Workshop має clean patterns у `.vscode/settings.json`. Також тимчасові файли виключені у `.gitignore`.

PDF не виключений з git, тому фінальні PDF-файли можна комітити за потреби.

Типові тимчасові файли:

- `*.aux`;
- `*.log`;
- `*.fdb_latexmk`;
- `*.fls`;
- `*.synctex.gz`;
- `*.toc`;
- `*.out`;
- `*.xdv`.

## Поради щодо роботи з шаблоном

- Для нового документа краще почати з копії `knu_methodical_elements_showcase.tex`.
- Не підключайте вручну `fontspec`, `polyglossia`, `geometry`, `caption`, `hyperref`, якщо використовуєте `knu-fcc-style.sty`: стиль уже робить це сам.
- Для структурних ненумерованих частин використовуйте `\KNUUnnumberedSection`, а не просто `\section*`, якщо розділ має потрапити до змісту.
- Для коректного `\KNUWorkStats` тримайте бібліографію через `\bibitem` або адаптуйте стиль під інший бібліографічний механізм.
- Перевіряйте фінальний PDF після зміни структури додатків, таблиць, рисунків або бібліографії.
