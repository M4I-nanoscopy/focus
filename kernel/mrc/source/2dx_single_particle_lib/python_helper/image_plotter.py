from image_plotter_func import *
	
	
def generatePerImagePlots(p_list, pdf_file):
	plotPositions(p_list, pdf_file)
	plotUsedForReconstruction(p_list, pdf_file)
	plotShifts(p_list, pdf_file)
	plotAngularChange(p_list, pdf_file)
	plotLastAngularChange(p_list, pdf_file)
	plotTLTAXIS(p_list, pdf_file)
	plotTLTANG(p_list, pdf_file)
	plotTAXA(p_list, pdf_file)
	plotSimilarity(p_list, pdf_file)
	plotQuality(p_list, pdf_file)
	plotConsistency(p_list, pdf_file)
	plotWeightAll(p_list, pdf_file)
	plotWeight(p_list, pdf_file)
#	plotContainerNumber(p_list, pdf_file)
	plotSimMeasureHist(p_list, pdf_file)
	plotHists(p_list, pdf_file)
	plotShiftDetails(p_list, pdf_file)
	
	
	
if __name__ == '__main__':

	if len(sys.argv) != 3:
		sys.exit("Usage: python anlge_plotter.py [filename] [filename out]")

	data = readFile(sys.argv[1])
	per_image_lists = generatePerImageLists(data)
	pp = PdfPages(sys.argv[2])
	
	for local_plist in per_image_lists:
		generatePerImagePlots(local_plist, pp)
	
	pp.close()
	