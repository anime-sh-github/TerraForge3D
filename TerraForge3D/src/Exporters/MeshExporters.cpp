#include "Exporters/ExportManager.h"
#include "Generators/MeshGeneratorManager.h"
#include "Data/ApplicationState.h"
#include "Base/Base.h"

#include "Data/Serializer.h"

#include <sstream>
#include <fstream>

#include <iostream>
#include <string>
#include <stdio.h>
#include <filesystem>
#include <time.h>



Mesh* ExportManager::ApplyMeshTransform(Mesh* mesh)
{
	for (int i = 0; i < mesh->vertexCount; i++)
	{
		const auto& vert = mesh->vert[i];
		auto tmp = 0.0f; appState->mainMap.currentTileDataLayers[0]->GetPixelF(vert.texCoord.x, vert.texCoord.y, &tmp);
		mesh->vert[i].position = vert.position + vert.normal * tmp;
	}
	return mesh;
}

bool ExportManager::ExportMesh(std::string path, Mesh* mesh, int format)
{
	auto& fsPath = std::filesystem::path(path);
	auto& filename0 = fsPath.filename().u8string();
	auto filename = std::string("");
	for (auto ch : filename0) if (std::isalnum(ch) || ch == ' ' || ch == '_') filename += ch;
	path = fsPath.parent_path().u8string() + "/" + filename;
	bool add_extension = fsPath.has_extension();
	switch (format)
	{
		case 0: path += ".obj"; return objExporter.Export(path, mesh, &this->exportProgress);
		case 1: path += ".stl"; return stlExporter.ExportASCII(path, mesh, &this->exportProgress);
		case 2: path += ".stl"; return stlExporter.ExportBinary(path, mesh, &this->exportProgress);
		case 3: path += ".ply"; return plyExporter.ExportASCII(path, mesh, &this->exportProgress);
		case 4: path += ".ply"; return plyExporter.ExportBinary(path, mesh, &this->exportProgress);
		case 5: path += ".dae"; return daeExporter.Export(path, mesh, &this->exportProgress);
		case 6: path += ".gltf"; return gltfExporter.ExportGLTF(path, path + ".bin", mesh, &this->exportProgress);
		default: return false;
	}
	return false;
}


void ExportManager::ExportMeshCurrentTile(std::string path, bool* exporting, int format)
{
	if (exporting) *exporting = true;
	auto& worker = std::thread([path, format, exporting, this]()->void {
		using namespace std::chrono_literals;
		this->SetStatusMessage("Exporting : " + path);
		appState->states.pauseUpdation = true; // disable updation from main thread
		while (appState->states.remeshing) /*std::this_thread::sleep_for(100ms)*/; // wait for current generation to finish
		appState->meshGenerator->StartGeneration(); // restart fresh generation 
		while (appState->states.remeshing) /*std::this_thread::sleep_for(100ms)*/; // wait for fresh generation to finish
		//auto originalMesh = ; 
		//;
		if (!this->ExportMesh(path, this->ApplyMeshTransform(appState->models.mainModel->mesh->Clone()), format)) this->SetStatusMessage("Failed to export : " + path);
		else this->SetStatusMessage("");
		appState->states.pauseUpdation = false; // enable updation from main thread
		this->exportProgress = 1.1f;
		if (exporting) *exporting = false;
		});
	worker.detach();
}

void ExportManager::ExportMeshAllTiles(std::string pathStr, bool* exporting, int format)
{
	if (exporting) *exporting = true;
	this->hideExportControls = true;
	auto& worker = std::thread([pathStr, format, exporting, this]()->void {
		using namespace std::chrono_literals;
		auto& path = std::filesystem::path(pathStr);
		auto& parentDir = path.parent_path().u8string();
		auto& filename = path.filename().u8string();
		auto& extension = std::string("");
		auto& outFilename = std::string("");
		bool exportingTile = false;
		if (path.has_extension())
		{
			filename = filename.substr(0, filename.find_last_of("."));
			extension = path.extension().u8string();
		}
		auto previousTileX = appState->mainMap.currentTileX;
		auto previousTileY = appState->mainMap.currentTileY;
		this->SetStatusMessage("");
		for(auto tx = 0; tx < appState->mainMap.tileCount; tx++)
		{
			for (auto ty = 0; ty < appState->mainMap.tileCount; ty++)
			{
				appState->mainMap.currentTileX = tx;
				appState->mainMap.currentTileY = ty;
				outFilename = parentDir + "/" + filename + "_" + std::to_string(tx) + "_" + std::to_string(ty) + extension;
				exportingTile = false; this->ExportMeshCurrentTile(outFilename, &exportingTile, format); // export tile mesh
				while (exportingTile) std::this_thread::sleep_for(100ms); // wait for the tile export to finish
			}
		}
		this->SetStatusMessage("");
		appState->mainMap.currentTileX = previousTileX;
		appState->mainMap.currentTileY = previousTileY;
		if (exporting) *exporting = false;
		this->hideExportControls = false;
		});
	worker.detach();
}