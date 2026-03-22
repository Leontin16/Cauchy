#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Лабораторная работа №1 — Численное решение задачи Коши
GUI-обёртка над C++ решением (lab1.exe / lab1)
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, font as tkfont
import subprocess
import os
import sys
import threading

import matplotlib
matplotlib.use('TkAgg')
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure
import numpy as np


# ─────────────────────────────────────────────────────────────
# Поиск C++ исполняемого файла
# ─────────────────────────────────────────────────────────────
def _find_exe() -> str | None:
    base = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    candidates = [
        os.path.join(base, 'lab1.exe'),
        os.path.join(base, 'lab1'),
        os.path.join(base, 'main.exe'),
        os.path.join(base, 'main'),
    ]
    # также проверим cwd
    for name in ('lab1.exe', 'lab1', 'main.exe', 'main'):
        candidates.append(os.path.join(os.getcwd(), name))
    for p in candidates:
        if os.path.isfile(p):
            return p
    return None


# ─────────────────────────────────────────────────────────────
# Главное окно
# ─────────────────────────────────────────────────────────────
class App(tk.Tk):
    BG = '#1e1e2e'
    FG = '#cdd6f4'
    ACCENT = '#89b4fa'
    BTN_BG = '#313244'
    ENTRY_BG = '#313244'
    SEP = '#45475a'
    GREEN = '#a6e3a1'
    RED = '#f38ba8'
    YELLOW = '#f9e2af'

    def __init__(self):
        super().__init__()
        self.title('Лаб. работа №1 — Задача Коши (РК4)')
        self.geometry('1150x720')
        self.minsize(900, 600)
        self.configure(bg=self.BG)

        self.exe_path = _find_exe()
        self._last_data: np.ndarray | None = None
        self._last_is_test: bool = True

        self._style()
        self._build()
        self._update_visibility()

    # ── Стили ──────────────────────────────────────────────
    def _style(self):
        s = ttk.Style(self)
        s.theme_use('clam')

        s.configure('.', background=self.BG, foreground=self.FG,
                    fieldbackground=self.ENTRY_BG, font=('Segoe UI', 10))
        s.configure('TFrame', background=self.BG)
        s.configure('TLabel', background=self.BG, foreground=self.FG)
        s.configure('TLabelframe', background=self.BG, foreground=self.ACCENT,
                    bordercolor=self.SEP)
        s.configure('TLabelframe.Label', background=self.BG, foreground=self.ACCENT,
                    font=('Segoe UI', 10, 'bold'))
        s.configure('TNotebook', background=self.BG, tabmargins=[2, 5, 2, 0])
        s.configure('TNotebook.Tab', background=self.BTN_BG, foreground=self.FG,
                    padding=[12, 4], font=('Segoe UI', 10))
        s.map('TNotebook.Tab', background=[('selected', self.ACCENT)],
              foreground=[('selected', self.BG)])
        s.configure('TRadiobutton', background=self.BG, foreground=self.FG)
        s.map('TRadiobutton', background=[('active', self.BG)])
        s.configure('Run.TButton', background=self.ACCENT, foreground=self.BG,
                    font=('Segoe UI', 11, 'bold'), padding=8)
        s.map('Run.TButton', background=[('active', self.GREEN), ('disabled', self.SEP)],
              foreground=[('disabled', self.FG)])
        s.configure('TEntry', fieldbackground=self.ENTRY_BG, foreground=self.FG,
                    insertcolor=self.FG)
        s.configure('TSeparator', background=self.SEP)

    # ── Разметка ────────────────────────────────────────────
    def _build(self):
        # Заголовок
        hdr = tk.Label(self, text='Численное решение задачи Коши методом Рунге-Кутта 4-го порядка',
                       bg=self.BG, fg=self.ACCENT, font=('Segoe UI', 12, 'bold'))
        hdr.pack(fill='x', padx=12, pady=(10, 4))

        ttk.Separator(self, orient='horizontal').pack(fill='x', padx=12, pady=2)

        # Основной PanedWindow
        paned = ttk.PanedWindow(self, orient='horizontal')
        paned.pack(fill='both', expand=True, padx=8, pady=6)

        left = ttk.Frame(paned, width=280)
        paned.add(left, weight=0)

        right = ttk.Frame(paned)
        paned.add(right, weight=1)

        self._build_left(left)
        self._build_right(right)

    # ── Левая панель: параметры ──────────────────────────────
    def _build_left(self, parent):
        parent.columnconfigure(0, weight=1)

        # --- Тип задачи ---
        lf1 = ttk.LabelFrame(parent, text=' Задача ')
        lf1.grid(row=0, column=0, sticky='ew', padx=6, pady=4)
        lf1.columnconfigure(0, weight=1)

        self.problem_var = tk.IntVar(value=1)
        ttk.Radiobutton(lf1, text='1 — Тестовая   du/dx = −1.5·u',
                        variable=self.problem_var, value=1,
                        command=self._update_visibility).grid(row=0, column=0, sticky='w', padx=6, pady=2)
        ttk.Radiobutton(lf1, text='2 — Основная   (осциллятор)',
                        variable=self.problem_var, value=2,
                        command=self._update_visibility).grid(row=1, column=0, sticky='w', padx=6, pady=2)

        # --- Метод ---
        lf2 = ttk.LabelFrame(parent, text=' Метод ')
        lf2.grid(row=1, column=0, sticky='ew', padx=6, pady=4)
        lf2.columnconfigure(0, weight=1)

        self.method_var = tk.IntVar(value=1)
        ttk.Radiobutton(lf2, text='1 — Постоянный шаг',
                        variable=self.method_var, value=1,
                        command=self._update_visibility).grid(row=0, column=0, sticky='w', padx=6, pady=2)
        ttk.Radiobutton(lf2, text='2 — Адаптивный шаг',
                        variable=self.method_var, value=2,
                        command=self._update_visibility).grid(row=1, column=0, sticky='w', padx=6, pady=2)

        # --- Параметры ---
        lf3 = ttk.LabelFrame(parent, text=' Параметры интегрирования ')
        lf3.grid(row=2, column=0, sticky='ew', padx=6, pady=4)
        lf3.columnconfigure(1, weight=1)

        fields = [
            ('a  (лев. граница):', 'a_var', '0'),
            ('b  (прав. граница):', 'b_var', '5'),
            ('h₀ (нач. шаг):', 'h_var', '0.001'),
            ('Nmax:', 'nmax_var', '10000'),
        ]
        for r, (lbl, attr, default) in enumerate(fields):
            ttk.Label(lf3, text=lbl).grid(row=r, column=0, sticky='w', padx=6, pady=3)
            var = tk.StringVar(value=default)
            setattr(self, attr, var)
            ttk.Entry(lf3, textvariable=var, width=12).grid(row=r, column=1, sticky='ew', padx=6, pady=3)

        # ε — только для адаптивного
        self.eps_lbl = ttk.Label(lf3, text='ε  (погрешность):')
        self.eps_lbl.grid(row=len(fields), column=0, sticky='w', padx=6, pady=3)
        self.eps_var = tk.StringVar(value='1e-5')
        self.eps_ent = ttk.Entry(lf3, textvariable=self.eps_var, width=12)
        self.eps_ent.grid(row=len(fields), column=1, sticky='ew', padx=6, pady=3)

        # u₀ — только для тестовой
        self.u0_lbl = ttk.Label(lf3, text='u₀ (нач. значение):')
        self.u0_lbl.grid(row=len(fields)+1, column=0, sticky='w', padx=6, pady=3)
        self.u0_var = tk.StringVar(value='1.0')
        self.u0_ent = ttk.Entry(lf3, textvariable=self.u0_var, width=12)
        self.u0_ent.grid(row=len(fields)+1, column=1, sticky='ew', padx=6, pady=3)

        # --- Кнопка ---
        self.run_btn = ttk.Button(parent, text='▶  Запустить расчёт',
                                  style='Run.TButton', command=self._on_run)
        self.run_btn.grid(row=3, column=0, sticky='ew', padx=6, pady=10)

        # Статус / путь к exe
        self.status_var = tk.StringVar()
        tk.Label(parent, textvariable=self.status_var, bg=self.BG, fg=self.YELLOW,
                 font=('Segoe UI', 9), wraplength=260, justify='left').grid(
            row=4, column=0, sticky='w', padx=6)

        if self.exe_path:
            self.status_var.set(f'Исполняемый файл:\n{os.path.basename(self.exe_path)}')
        else:
            self.status_var.set('⚠ lab1.exe не найден!\nПоложите его рядом с этим скриптом.')

    # ── Правая панель: результаты ───────────────────────────
    def _build_right(self, parent):
        nb = ttk.Notebook(parent)
        nb.pack(fill='both', expand=True)

        # Вкладка 1: консольный вывод
        tab_txt = ttk.Frame(nb)
        nb.add(tab_txt, text='  Таблица / Статистика  ')
        self.output_txt = scrolledtext.ScrolledText(
            tab_txt,
            font=('Courier New', 9),
            bg='#11111b', fg=self.FG,
            insertbackground=self.FG,
            wrap='none',
            relief='flat',
            state='disabled'
        )
        # горизонтальный скролл
        xscroll = ttk.Scrollbar(tab_txt, orient='horizontal',
                                command=self.output_txt.xview)
        self.output_txt.configure(xscrollcommand=xscroll.set)
        xscroll.pack(side='bottom', fill='x')
        self.output_txt.pack(fill='both', expand=True)

        # Вкладка 2: графики решения
        tab_plot = ttk.Frame(nb)
        nb.add(tab_plot, text='  График решения  ')
        self.fig1 = Figure(facecolor='#11111b', tight_layout=True)
        self.ax1 = self.fig1.add_subplot(111)
        self._style_axes(self.ax1)
        self.canvas1 = FigureCanvasTkAgg(self.fig1, master=tab_plot)
        tb1 = NavigationToolbar2Tk(self.canvas1, tab_plot)
        tb1.config(bg=self.BTN_BG)
        tb1.pack(side='bottom', fill='x')
        self.canvas1.get_tk_widget().pack(fill='both', expand=True)

        # Вкладка 3: фазовый портрет
        tab_phase = ttk.Frame(nb)
        nb.add(tab_phase, text='  Фазовый портрет  ')
        self.fig2 = Figure(facecolor='#11111b', tight_layout=True)
        self.ax2 = self.fig2.add_subplot(111)
        self._style_axes(self.ax2)
        self.canvas2 = FigureCanvasTkAgg(self.fig2, master=tab_phase)
        tb2 = NavigationToolbar2Tk(self.canvas2, tab_phase)
        tb2.config(bg=self.BTN_BG)
        tb2.pack(side='bottom', fill='x')
        self.canvas2.get_tk_widget().pack(fill='both', expand=True)

        # Вкладка 4: погрешность
        tab_err = ttk.Frame(nb)
        nb.add(tab_err, text='  Погрешность  ')
        self.fig3 = Figure(facecolor='#11111b', tight_layout=True)
        self.ax3 = self.fig3.add_subplot(111)
        self._style_axes(self.ax3)
        self.canvas3 = FigureCanvasTkAgg(self.fig3, master=tab_err)
        tb3 = NavigationToolbar2Tk(self.canvas3, tab_err)
        tb3.config(bg=self.BTN_BG)
        tb3.pack(side='bottom', fill='x')
        self.canvas3.get_tk_widget().pack(fill='both', expand=True)

        self._nb = nb

    def _style_axes(self, ax):
        ax.set_facecolor('#1e1e2e')
        ax.tick_params(colors=self.FG, labelsize=9)
        ax.xaxis.label.set_color(self.FG)
        ax.yaxis.label.set_color(self.FG)
        ax.title.set_color(self.ACCENT)
        for spine in ax.spines.values():
            spine.set_color(self.SEP)

    # ── Видимость полей ─────────────────────────────────────
    def _update_visibility(self):
        is_adaptive = self.method_var.get() == 2
        is_test = self.problem_var.get() == 1

        state_eps = 'normal' if is_adaptive else 'disabled'
        self.eps_ent.configure(state=state_eps)
        self.eps_lbl.configure(foreground=self.FG if is_adaptive else self.SEP)

        state_u0 = 'normal' if is_test else 'disabled'
        self.u0_ent.configure(state=state_u0)
        self.u0_lbl.configure(foreground=self.FG if is_test else self.SEP)

    # ── Запуск ──────────────────────────────────────────────
    def _on_run(self):
        if not self.exe_path:
            messagebox.showerror('Ошибка', 'Исполняемый файл не найден.\n'
                                 'Скомпилируйте lab1.exe и положите рядом с gui.py')
            return
        self.run_btn.configure(state='disabled')
        self.status_var.set('⏳ Выполняется расчёт...')
        threading.Thread(target=self._worker, daemon=True).start()

    def _worker(self):
        try:
            inputs = [
                str(self.problem_var.get()),
                str(self.method_var.get()),
                self.a_var.get(),
                self.b_var.get(),
                self.h_var.get(),
            ]
            if self.method_var.get() == 2:
                inputs.append(self.eps_var.get())
            inputs.append(self.nmax_var.get())
            if self.problem_var.get() == 1:
                inputs.append(self.u0_var.get())

            stdin_data = '\n'.join(inputs) + '\n'

            proc = subprocess.run(
                [self.exe_path],
                input=stdin_data,
                capture_output=True,
                text=True,
                encoding='utf-8',
                timeout=60,
                cwd=os.path.dirname(os.path.abspath(self.exe_path))
            )
            output = proc.stdout
            if proc.stderr:
                output += '\n[stderr]\n' + proc.stderr

            self.after(0, self._on_done, output, True)

        except subprocess.TimeoutExpired:
            self.after(0, self._on_done, '❌ Ошибка: превышено время выполнения (60 с)', False)
        except Exception as ex:
            self.after(0, self._on_done, f'❌ Ошибка: {ex}', False)

    def _on_done(self, text: str, ok: bool):
        self.run_btn.configure(state='normal')
        self.status_var.set('✔ Готово' if ok else '❌ Ошибка')

        self.output_txt.configure(state='normal')
        self.output_txt.delete('1.0', 'end')
        self.output_txt.insert('end', text)
        self.output_txt.configure(state='disabled')

        if ok:
            self._update_plots()

    # ── Графики ─────────────────────────────────────────────
    def _update_plots(self):
        try:
            self._update_plots_inner()
        except Exception as e:
            messagebox.showerror('Ошибка построения графиков', str(e))

    def _update_plots_inner(self):
        is_test = self.problem_var.get() == 1
        method = self.method_var.get()

        fname = (
            'test_const.txt' if is_test and method == 1 else
            'test_adapt.txt' if is_test and method == 2 else
            'main_const.txt' if not is_test and method == 1 else
            'main_adapt.txt'
        )

        # Ищем файл рядом с exe, в cwd и рядом со скриптом
        exe_dir = os.path.dirname(os.path.abspath(self.exe_path))
        candidates = [
            os.path.join(exe_dir, fname),
            os.path.join(os.getcwd(), fname),
            os.path.join(os.path.dirname(os.path.abspath(__file__)), fname),
        ]
        fpath = next((p for p in candidates if os.path.isfile(p)), None)
        if fpath is None:
            messagebox.showwarning(
                'Файл не найден',
                f'Не удалось найти файл данных: {fname}\n\nИскал в:\n' +
                '\n'.join(candidates)
            )
            return

        rows = []
        with open(fpath, encoding='utf-8', errors='replace') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                try:
                    rows.append([float(x) for x in line.split()])
                except ValueError:
                    pass

        if not rows:
            messagebox.showwarning('Нет данных', f'Файл {fname} пуст или не содержит числовых данных.')
            return

        data = np.array(rows)

        # Filter out rows with NaN or Inf — signal numerical blow-up
        valid = np.all(np.isfinite(data), axis=1)
        n_bad = np.sum(~valid)
        if n_bad > 0:
            messagebox.showwarning(
                'Численная нестабильность',
                f'В данных обнаружено {n_bad} строк с NaN/Inf.\n'
                f'Уменьшите шаг h (рекомендуется h ≤ 0.001 для основной задачи).\n'
                f'Графики построены по {np.sum(valid)} корректным точкам.'
            )
            data = data[valid]
        if data.shape[0] == 0:
            messagebox.showerror('Нет данных', 'Все точки содержат NaN/Inf. Уменьшите шаг h.')
            return

        self._last_data = data
        self._last_is_test = is_test

        # Очистка
        for ax in (self.ax1, self.ax2, self.ax3):
            ax.clear()
            self._style_axes(ax)

        if is_test:
            # cols: x  v  v2  err  h  exact
            x = data[:, 0]
            v = data[:, 1]       # приближённое (шаг h)
            exact = data[:, 5]   # точное

            # График решения
            self.ax1.plot(x, exact, color=self.GREEN, lw=2, label='Точное u(x)')
            self.ax1.plot(x, v, 'o--', color=self.ACCENT, lw=1.5,
                          markersize=4, label='Приближённое v(x)')
            self.ax1.set_xlabel('x')
            self.ax1.set_ylabel('u, v')
            self.ax1.set_title('Тестовая задача: du/dx = −1.5u')
            self.ax1.legend(facecolor='#313244', edgecolor=self.SEP, labelcolor=self.FG)
            self.ax1.grid(True, color=self.SEP, linestyle='--', linewidth=0.5)

            # Фазовый портрет — не применим
            self.ax2.text(0.5, 0.5, 'Фазовый портрет\nдоступен только\nдля основной задачи',
                          ha='center', va='center', transform=self.ax2.transAxes,
                          fontsize=12, color=self.SEP)

            # Погрешность
            err = np.abs(data[:, 3])   # |v - v2|
            glob = np.abs(exact - v)   # |u - v|
            self.ax3.semilogy(x, err, color=self.YELLOW, lw=1.5, label='ОЛП  |v − v2|')
            self.ax3.semilogy(x, glob, color=self.RED, lw=1.5, linestyle='--',
                              label='|u − v|  (глобальная)')
            self.ax3.set_xlabel('x')
            self.ax3.set_ylabel('погрешность (log)')
            self.ax3.set_title('Оценка погрешности')
            self.ax3.legend(facecolor='#313244', edgecolor=self.SEP, labelcolor=self.FG)
            self.ax3.grid(True, color=self.SEP, linestyle='--', linewidth=0.5, which='both')

        else:
            # cols: x  u  v_velocity  u_h  u_h2  err  h
            x = data[:, 0]
            u = data[:, 1]        # смещение
            vel = data[:, 2]      # скорость

            # График решения u(x)
            self.ax1.plot(x, u, color=self.ACCENT, lw=2, label='u(x) — смещение')
            self.ax1.set_xlabel('x  (время, с)')
            self.ax1.set_ylabel('u, см')
            self.ax1.set_title('Основная задача: нелинейный осциллятор')
            self.ax1.legend(facecolor='#313244', edgecolor=self.SEP, labelcolor=self.FG)
            self.ax1.grid(True, color=self.SEP, linestyle='--', linewidth=0.5)

            # Фазовый портрет u' = f(u)
            self.ax2.plot(u, vel, color=self.GREEN, lw=1.5)
            self.ax2.set_xlabel("u, см")
            self.ax2.set_ylabel("u', см/с")
            self.ax2.set_title('Фазовый портрет')
            self.ax2.grid(True, color=self.SEP, linestyle='--', linewidth=0.5)

            # Погрешность
            err = np.abs(data[:, 5])   # |v - v2|
            self.ax3.semilogy(x, err + 1e-20, color=self.YELLOW, lw=1.5, label='ОЛП  |v − v2|')
            self.ax3.set_xlabel('x')
            self.ax3.set_ylabel('ОЛП (log)')
            self.ax3.set_title('Оценка локальной погрешности')
            self.ax3.legend(facecolor='#313244', edgecolor=self.SEP, labelcolor=self.FG)
            self.ax3.grid(True, color=self.SEP, linestyle='--', linewidth=0.5, which='both')

        for canvas in (self.canvas1, self.canvas2, self.canvas3):
            canvas.draw()


# ─────────────────────────────────────────────────────────────
if __name__ == '__main__':
    app = App()
    app.mainloop()
