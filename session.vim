let SessionLoad = 1
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/git/my/appdog
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +15 include/appdog.h
badd +32 src/appdogd.cpp
badd +37 src/appdogd.h
badd +21 include/client.h
badd +28 src/client.cpp
badd +17 CMakeLists.txt
badd +18 src/client_.h
badd +28 src/client_.cpp
badd +1 src/messages/message.h
badd +14 src/messages/activate.h
badd +14 src/messages/deactivate.h
badd +13 src/messages/confirm.h
badd +6 src/messages/kick.h
badd +24 src/messages/list.h
badd +1 NERD_tree_1
argglobal
%argdel
edit NERD_tree_1
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd _ | wincmd |
split
1wincmd k
wincmd w
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winminheight=0
set winheight=1
set winminwidth=0
set winwidth=1
exe '1resize ' . ((&lines * 124 + 64) / 129)
exe 'vert 1resize ' . ((&columns * 139 + 139) / 278)
exe '2resize ' . ((&lines * 1 + 64) / 129)
exe 'vert 2resize ' . ((&columns * 139 + 139) / 278)
exe 'vert 3resize ' . ((&columns * 138 + 139) / 278)
exe '4resize ' . ((&lines * 12 + 64) / 129)
exe '5resize ' . ((&lines * 12 + 64) / 129)
argglobal
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal nofen
silent! normal! zE
let s:l = 1 - ((0 * winheight(0) + 62) / 124)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1
normal! 0
wincmd w
argglobal
if bufexists("src/client_.h") | buffer src/client_.h | else | edit src/client_.h | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 2 - ((0 * winheight(0) + 0) / 1)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
2
normal! 0
wincmd w
argglobal
if bufexists("src/messages/activate.h") | buffer src/messages/activate.h | else | edit src/messages/activate.h | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 10 - ((9 * winheight(0) + 63) / 126)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
10
normal! 03|
wincmd w
argglobal
if bufexists("NERD_tree_1") | buffer NERD_tree_1 | else | edit NERD_tree_1 | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal nofen
silent! normal! zE
let s:l = 1 - ((0 * winheight(0) + 6) / 12)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1
normal! 0
wincmd w
argglobal
if bufexists("NERD_tree_1") | buffer NERD_tree_1 | else | edit NERD_tree_1 | endif
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal nofen
silent! normal! zE
let s:l = 1 - ((0 * winheight(0) + 6) / 12)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
1
normal! 0
wincmd w
exe '1resize ' . ((&lines * 124 + 64) / 129)
exe 'vert 1resize ' . ((&columns * 139 + 139) / 278)
exe '2resize ' . ((&lines * 1 + 64) / 129)
exe 'vert 2resize ' . ((&columns * 139 + 139) / 278)
exe 'vert 3resize ' . ((&columns * 138 + 139) / 278)
exe '4resize ' . ((&lines * 12 + 64) / 129)
exe '5resize ' . ((&lines * 12 + 64) / 129)
tabnext 1
if exists('s:wipebuf') && getbufvar(s:wipebuf, '&buftype') isnot# 'terminal'
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 winminheight=1 winminwidth=1 shortmess=filnxtToOcsF
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
