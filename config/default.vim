syntax match KessCPrintf /printf/
syntax match KessCUint8 /uint8/
syntax match KessCInt /[0-9]\+/
syntax match KessStringLit /\v(['"])%(\1@![^\\]|\\.)*\1/


highlight KessCPrintf ctermfg=220 guifg=#ffd700
highlight KessCUint8 ctermfg=220 guifg=#ffd700
highlight KessCInt ctermfg=205 guifg=#ff5faf
highlight KessStringLit ctermfg=85 guifg=#5fffaf
