#!/usr/bin/env python3

import os
import sys
from PIL import Image
import imagehash

failed = False

def compute_hash(image_path):
    return imagehash.whash(Image.open(image_path))

def compare_images(image_folder, ground_truth_path):
    global failed
    # Compute hash for the ground truth image
    ground_truth_hash = compute_hash(ground_truth_path)
    
    # Generate list of image paths in the given folder
    image_paths = [os.path.join(image_folder, file) for file in os.listdir(image_folder) if file.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.bmp'))]
    
    # Compare each image with the ground truth
    for image_path in image_paths:
        image_hash = compute_hash(image_path)
        hamming_distance = image_hash - ground_truth_hash
        similarity_percentage = 100 * (1 - (hamming_distance / len(str(ground_truth_hash))))

        # Fail if `similarity_percentage` is below 90%
        if not failed:
            failed = similarity_percentage < 90.0

        print(f"Image: {image_path}, Similarity: {similarity_percentage:.2f}%")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <image_folder> <ground_truth_image>")
        sys.exit(1)
    
    # Get folder path and ground truth image path from command line arguments
    image_folder = sys.argv[1]
    ground_truth_path = sys.argv[2]
    
    # Compare images
    compare_images(image_folder, ground_truth_path)

    if failed:
        print("Some formats failed the similarity score >90%")
        sys.exit(1)
