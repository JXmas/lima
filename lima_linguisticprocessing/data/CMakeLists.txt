#   Copyright 2002-2013 CEA LIST
#    
#   This file is part of LIMA.
#
#   LIMA is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   LIMA is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License
#   along with LIMA.  If not, see <http://www.gnu.org/licenses/>

########### install files ###############

install(FILES
    test-eng.abbrev.xml 
    test-eng.default.xml 
    test-eng.disambiguated.xml 
    test-eng.hyphen.xml 
    test-eng.idiom.xml
    test-eng.sa.xml 
    test-eng.se.xml 
    test-eng.simpleword.xml 
    test-eng.tokenizer.xml
    test-fre.default.xml 
    test-fre.disambiguated.xml 
    test-fre.hyphen.xml   
    test-fre.idiom.xml 
    test-fre.sa.xml 
    test-fre.se.xml 
    test-fre.simpleword.xml
    test-fre.tokenizer.xml 
    #test-fre.se-beforepos.xml 
    test-fre.multilevel.xml 
  DESTINATION share/apps/lima/tests)
