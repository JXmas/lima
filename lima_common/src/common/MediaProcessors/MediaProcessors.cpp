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
/***************************************************************************
 *   Copyright (C) 2004-2012 by CEA LIST                                   *
 *                                                                         *
 ***************************************************************************/

#include "MediaProcessors.h"

#include "MediaProcessUnit.h"
#include "MediaAnalysisDumper.h"

#include "common/XMLConfigurationFiles/moduleConfigurationStructure.h"
#include "common/XMLConfigurationFiles/xmlConfigurationFileExceptions.h"
#include "common/MediaticData/mediaticData.h"
#include <iostream>


using namespace std;
using namespace Lima::Common::XMLConfigurationFiles;
using namespace Lima::Common::MediaticData;

namespace Lima {


class MediaProcessorsPrivate
{
  friend class MediaProcessors;

  MediaProcessorsPrivate();
  MediaProcessorsPrivate(const MediaProcessorsPrivate& /*mpp*/) {}
  virtual ~MediaProcessorsPrivate() {}

  std::map<MediaId,MediaProcessUnit::Manager*> m_pipelineManagers;
  std::map<std::string,std::map<MediaId,const MediaProcessUnitPipeline*> > m_pipelines;

//   std::map<MediaId,MediaAnalysisDumper::Manager*> m_dumperManagers;
  std::map<std::string,std::map<MediaId,const MediaAnalysisDumper*> > m_analysisDumpers;

  // recursive read of configuration files
  void includeProcessors(Common::XMLConfigurationFiles::ModuleConfigurationStructure& module,
                          Common::XMLConfigurationFiles::ModuleConfigurationStructure& newModule);


};

MediaProcessorsPrivate::MediaProcessorsPrivate() :
    m_pipelineManagers(), m_pipelines(), m_analysisDumpers()
{
}

MediaProcessors::MediaProcessors() : Singleton<MediaProcessors>(), m_d(new MediaProcessorsPrivate()) {}
MediaProcessors::MediaProcessors(const MediaProcessors& mp) : Singleton<MediaProcessors>(), m_d(new MediaProcessorsPrivate(*mp.m_d)) {}

MediaProcessors::~MediaProcessors()
{

    PROCESSORSLOGINIT;

    LWARN << "MediaProcessors::~MediaProcessors() delete MediaProcessors ";
    for ( std::map<MediaId,MediaProcessUnit::Manager*>::iterator it=m_d->m_pipelineManagers.begin();
            it!=m_d->m_pipelineManagers.end();
            it++ )
    {
        LWARN <<  "delete " << it->first;
        delete it->second;
        it->second=0;
    }
    //   for (std::map<MediaId,MediaAnalysisDumper::Manager*>::iterator it=m_dumperManagers.begin();
    //        it!=m_dumperManagers.end();
    //        it++)
    //   {
    //     delete it->second;
    //     it->second=0;
    //   }
    delete m_d;
}

void MediaProcessors::initMedia (
    MediaId med,
    Common::XMLConfigurationFiles::ModuleConfigurationStructure& confProc )
{
  PROCESSORSLOGINIT;
  LDEBUG << "MediaProcessors::initMedia" << med;

  MediaProcessUnitInitializationParameters lpuinit;
  lpuinit.media=med;
  m_d->m_pipelineManagers[med]=new MediaProcessUnit::Manager ( confProc,lpuinit );

  //   MediaAnalysisDumperInitializationParameters ladinit;
  //   ladinit.media=med;
  //   m_dumperManagers[lang]=new MediaAnalysisDumper::Manager(confDump,ladinit);

  // Recursive init for 'Processors'
  Common::XMLConfigurationFiles::ModuleConfigurationStructure&
  module=m_d->m_pipelineManagers[med]->getModuleConfigurationStructure();
  m_d->includeProcessors(module,module);
}

void MediaProcessors::initPipelines (
    Common::XMLConfigurationFiles::GroupConfigurationStructure& confGroup,
    const std::deque<std::string>& pipelines )
{
    PROCESSORSLOGINIT;
    deque<string> pipelinesToInit ( pipelines );
    map<string,map<string,string> > allpipelines;
    try
    {
        allpipelines=confGroup.getMaps();
    }
    catch ( NoSuchGroup& e )
    {
        LDEBUG << "no group 'declaration' to configure linguistic pipelines ";
        throw InvalidConfiguration(e.what());
    }

    // if pipelines is empty, then initialize all pipelines
    if ( pipelinesToInit.empty() )
    {
        for ( map<string,map<string,string> >::const_iterator mapItr=allpipelines.begin();
                mapItr!=allpipelines.end();
                mapItr++ )
        {
            pipelinesToInit.push_back ( mapItr->first );
        }
    }

    // init pipelines for all available medias
    for ( deque<string>::const_iterator pipItr=pipelinesToInit.begin();
            pipItr!=pipelinesToInit.end();
            pipItr++ )
    {
        //      std::cout << "initialize pipeline type  '" << *pipItr << "'" << std::endl;
        const map<string,string>& pipelineMapping=allpipelines[*pipItr];
        if ( pipelineMapping.empty() )
        {
            std::cout << "no pipeline '" << *pipItr << "' defined in pipeline declaration module ! " << std::endl;
            //         continue;
        }
        for ( std::map<MediaId,MediaProcessUnit::Manager*>::const_iterator mapItr=m_d->m_pipelineManagers.begin();
                mapItr!=m_d->m_pipelineManagers.end();
                mapItr++ )
        {
            const std::string& mediaStr=MediaticData::single().getMediaId ( mapItr->first );
            map<string,string>::const_iterator entryItr=pipelineMapping.find ( mediaStr );
            if ( entryItr==pipelineMapping.end() )
            {
                std::cout << "no pipeline '" << *pipItr << "' for media " << mediaStr << std::endl;
                //         continue;
            }
            const MediaProcessUnit* pu=mapItr->second->getObject ( entryItr->second );
            const MediaProcessUnitPipeline* pipeline=static_cast<const MediaProcessUnitPipeline*> ( pu );
            if ( pipeline==0 )
            {
                std::cout << "pipeline '" << *pipItr << "' for media " << mediaStr << " is not of type ProcessUnitPipeline !" << std::endl;
                //         throw InvalidConfiguration();
            }
            m_d->m_pipelines[*pipItr][mapItr->first]=pipeline;
        }
    }
}


const MediaProcessUnitPipeline* MediaProcessors::getPipelineForId ( MediaId med,const std::string& id ) const
{
    PROCESSORSLOGINIT;

    map<string,map<MediaId,const MediaProcessUnitPipeline*> >::const_iterator pipItr=m_d->m_pipelines.find ( id );
    if ( pipItr==m_d->m_pipelines.end() )
    {
        LWARN << "no pipeline type '" << id << "' defined in media " << med << " configuration file !";
        return 0;
    }
    map<MediaId,const MediaProcessUnitPipeline*>::const_iterator medItr=pipItr->second.find ( med );
    if ( medItr==pipItr->second.end() )
    {
      LWARN << "no media [" << med << ":" << MediaticData::single().getMediaId(med).c_str() << "] defined for pipeline '" << id.c_str() << "' in the configuration file !";
        return 0;
    }
    return medItr->second;
}

const MediaAnalysisDumper* MediaProcessors::getAnalysisDumperForId ( MediaId med,const std::string& type ) const
{
    PROCESSORSLOGINIT;
    std::map<std::string,std::map<MediaId,const MediaAnalysisDumper*> >::const_iterator dumpItr=m_d->m_analysisDumpers.find ( type );
    if ( dumpItr==m_d->m_analysisDumpers.end() )
    {
      LWARN << "no dumper type '" << type << "' defined in media "<<med<<" configuration file ";
        return 0;
    }
    map<MediaId,const MediaAnalysisDumper*>::const_iterator medItr=dumpItr->second.find ( med );
    if ( medItr==dumpItr->second.end() )
    {
      LWARN << "no media [" << med << ":" << MediaticData::single().getMediaId(med) << "] defined for dumper '" << type << "' in the configuration file !";
        return 0;
    }
    return medItr->second;
}

// two arguments: first is the module to modify, second is the module in which
// the 'include' group can be found : both arguments can be the same
void MediaProcessorsPrivate::
includeProcessors(Common::XMLConfigurationFiles::ModuleConfigurationStructure& module,
                  Common::XMLConfigurationFiles::ModuleConfigurationStructure& includeModule)
{
    try {
        deque<string> includeList=includeModule.getListValuesAtKeyOfGroupNamed("includeList","include");
        for (deque<string>::const_iterator it=includeList.begin(),
                it_end=includeList.end(); it!=it_end; it++) {
            string::size_type i=(*it).find("/");
            if (i==string::npos) {
                //PROCESSORSLOGINIT;
                //LERROR << "Cannot include processors " << *it
                //       << ": must specify file and module name";
                continue;
            }
            string fileName(""),moduleName("");
            try {
                //PROCESSORSLOGINIT;
                //LDEBUG << "i="<< i;
                fileName=Common::MediaticData::MediaticData::single().getConfigPath()+
                         "/"+string((*it),0,i);
                //LDEBUG << "filename="<< fileName;
                moduleName=string((*it),i+1);
                //LDEBUG << "moduleName="<< moduleName;
                XMLConfigurationFileParser parser(fileName);
                ModuleConfigurationStructure& newMod=parser.getModuleConfiguration(moduleName);
                module.addModule(newMod);
                //LDEBUG << "added module";
                ostringstream oss;
                for (ModuleConfigurationStructure::const_iterator it=module.begin(),it_end=module.end();
                        it!=it_end; it++) {
                    oss << (*it).first << ";";
                }
                //LDEBUG << "module contain following groups: " << oss.str();
                // recursive inclusions

                // 'include' group is not inserted in module by addModule function because a group
                // with same name was already there : use the one in newMod
                includeProcessors(module,newMod);
            }
            catch (NoSuchModule& ) {
                //PROCESSORSLOGINIT;
                //LERROR << "Cannot find module " << moduleName
                //       << " in file " << fileName;
            }
            catch (std::exception& ) {
                //PROCESSORSLOGINIT;
                //LERROR << "Error trying to find module " << moduleName
                //       << " in file " << fileName << ":" << e.what();
            }
        }
    }
    catch (NoSuchList& ) { } // do nothing: optional
    catch (NoSuchGroup& ) { } // do nothing: optional
}


} // Lima
