# SConstruct for ChineseCheckers program

import os

obj_env = Environment()
obj_env.MergeFlags( '!wx-config --cxxflags' )
obj_env.MergeFlags( '!pkg-config --cflags --libs libmongoc-1.0' )
obj_env.MergeFlags( '!pkg-config --cflags --libs libbson-1.0' )
#obj_env.Append( CCFLAGS = '--std=c++11' )
obj_env.Append( CCFLAGS = '-g' )

if not obj_env.GetOption( 'clean' ):
  conf = Configure( obj_env )

  #if not conf.CheckLib( 'wx_baseu-3.0' ):
  #  print( 'wxWidgets may not be installed; couldn\'t find it.' )
  #  Exit(1)

  #if not conf.CheckLib( 'SDL2' ):
  #  print( 'SDL2 may not be installed; could\'t find it.' )
  #  Exit(1)
  
  obj_env = conf.Finish()

cpp_source_list = Glob( 'Code/*.cpp' ) + Glob( 'Code/c3ga/*.cpp' )
h_source_list = Glob( 'Code/*.h' ) + Glob( 'Code/c3ga/*.h' )
source_list = cpp_source_list + h_source_list

object_list = []
for source_file in cpp_source_list:
  object_file = obj_env.StaticObject( source_file )
  object_list.append( object_file )

prog_env = Environment( PROGNAME = 'ChineseCheckers', parse_flags = '!wx-config --libs core base adv net gl aui richtext' )
prog_env.Append( LIBS = '-lGL' )
prog_env.Append( LIBS = '-lGLU' )
prog_env.Append( LIBS = '-lSDL2' )
#prog_env.Append( CCFLAGS = '-g' )
prog_env.MergeFlags( '!pkg-config --libs libbson-1.0' )
prog_env.MergeFlags( '!pkg-config --libs libmongoc-1.0' )
prog = prog_env.Program( '$PROGNAME', source = object_list )

dest_dir = '/usr/local'
if 'DESTDIR' in os.environ:
  dest_dir = os.environ[ 'DESTDIR' ]

install_env = Environment(
  BIN = dest_dir + '/bin',
  SHARE = dest_dir + '/share' )

sound_list = Glob( 'Sounds/*.wav' )

install_env.Install( '$BIN', prog )
install_env.Install( '$SHARE/ChiChe/Sounds', sound_list )
install_env.Alias( 'install', [ '$BIN', '$SHARE/ChiChe/Sounds' ] )
