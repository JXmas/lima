/*
    Copyright 2002-2013 CEA LIST

    This file is part of LIMA.

    LIMA is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LIMA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BagOfWordsTest
#include <boost/test/unit_test.hpp>

#include "linguisticProcessing/common/BagOfWords/bowToken.h"
#include "linguisticProcessing/common/BagOfWords/bowBinaryReaderWriter.h"

#include <sstream>

using namespace Lima;
using namespace Lima::Common::BagOfWords;

// Binary reading and writing

BOOST_AUTO_TEST_CASE( BagOfWordsTest1_1 )
{
  BoWToken bt1(QString::fromUtf8("lemma"), 1, 10, 5);
  BoWBinaryWriter writer;
  std::stringstream stream;
  std::map<BoWToken*,uint64_t> refMap;
  writer.writeBoWToken(stream, &bt1, refMap);

  BoWBinaryReader reader;
  std::vector<BoWToken*> rrefMap;
  BoWToken* bt2 = reader.readBoWToken(stream, rrefMap);

  BOOST_REQUIRE( bt1.getLemma() == bt2->getLemma() );
  delete bt2;
}

BOOST_AUTO_TEST_CASE( BagOfWordsTest1_2 )
{
  BoWToken bt1(QString::fromUtf8("démocratique"), 1, 10, 12);
  BoWBinaryWriter writer;
  std::stringstream stream;
  std::map<BoWToken*,uint64_t> refMap;
  writer.writeBoWToken(stream, &bt1, refMap);

  BoWBinaryReader reader;
  std::vector<BoWToken*> rrefMap;
  BoWToken* bt2 = reader.readBoWToken(stream, rrefMap);

  BOOST_REQUIRE( bt1.getLemma() == bt2->getLemma() );
  delete bt2;
}

BOOST_AUTO_TEST_CASE( BagOfWordsTest1_3 )
{
  BoWToken bt1(QString::fromUtf8("lemma"), 1, 10, 5);
  bt1.setInflectedForm(QString::fromUtf8("lemma"));
  BoWToken bt2(QString::fromUtf8("démocratique"), 1, 10, 12);
  bt2.setInflectedForm(QString::fromUtf8("démocratique"));
  BoWToken bt3(QString::fromUtf8("word"), 1, 10, 4);
  bt3.setInflectedForm(QString::fromUtf8("word"));
  BoWBinaryWriter writer;
  std::stringstream stream;
  std::map<BoWToken*,uint64_t> refMap;
  writer.writeBoWToken(stream, &bt1, refMap);
  writer.writeBoWToken(stream, &bt2, refMap);
  writer.writeBoWToken(stream, &bt3, refMap);

  BoWBinaryReader reader;
  std::vector<BoWToken*> rrefMap;
  BoWToken* rbt1 = reader.readBoWToken(stream, rrefMap);
  BoWToken* rbt2 = reader.readBoWToken(stream, rrefMap);
  BoWToken* rbt3 = reader.readBoWToken(stream, rrefMap);

  BOOST_REQUIRE( bt2.getLemma() == rbt2->getLemma() );
  delete rbt1;
  delete rbt2;
  delete rbt3;
}

