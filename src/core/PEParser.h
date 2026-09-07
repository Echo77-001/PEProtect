#pragma once

#include<vector>
#include<Windows.h>

#include"BasicPE.h"
#include"Utils.h"

#include <iostream>
#include <map>

class PEParser : public BasicPE {
private:

    std::vector<DLLtoImport> originalIAT;
    
    const std::vector<SectionInfo> parseSectionList(bool logSection = 0) const;

public:

    DWORD getVaFromExportTable(const char* funcName);
    
    SectionInfo& getSectionFromRVA(DWORD RVA);
    
	void read(const std::string& path, bool logSection = 0);

    ImportFunctionInfo getImportFuncFromRVA(DWORD RVA);

    const std::vector<DLLtoImport> getOriginalIAT() const;
};
