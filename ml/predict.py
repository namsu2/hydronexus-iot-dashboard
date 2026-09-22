#!/usr/bin/env python3
"""Predict the growth stage of a lettuce plant from one image.

The script can load a TorchScript or regular PyTorch checkpoint. For a
beginner-friendly first run, if a checkpoint is not available it falls back to
a small OpenCV heuristic and prints a warning. Replace the checkpoint with
your fine-tuned model before using this for a scientific measurement.

Expected model output: three class scores in this order:
    0 = seedling, 1 = vegetative, 2 = mature

Example:
    python ml/predict.py --image ./samples/lettuce.jpg
    python ml/predict.py --image ./samples/lettuce.jpg --model ./models/lettuce_stage.pt
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

import cv2
import numpy as np

STAGES = ["seedling", "vegetative", "mature"]
IMAGE_SIZE = 224


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Classify a lettuce growth stage.")
    parser.add_argument("--image", required=True, type=Path, help="Path to the input lettuce image.")
    parser.add_argument(
        "--model",
        type=Path,
        default=Path("models/lettuce_stage.pt"),
        help="TorchScript or PyTorch checkpoint path (default: models/lettuce_stage.pt).",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Print a machine-readable JSON object instead of a sentence.",
    )
    return parser.parse_args()


def load_image(path: Path) -> tuple[np.ndarray, np.ndarray]:
    image_bgr = cv2.imread(str(path))
    if image_bgr is None:
        raise FileNotFoundError(f"Could not read image: {path}")
    image_rgb = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2RGB)
    return image_bgr, image_rgb


def load_torch_model(path: Path) -> Any | None:
    """Load a TorchScript or PyTorch model when torch is installed."""
    if not path.exists():
        return None

    try:
        import torch
    except ImportError as error:
        raise RuntimeError(
            "A model file was supplied, but PyTorch is not installed. "
            "Run: pip install torch torchvision"
        ) from error

    try:
        model = torch.jit.load(str(path), map_location="cpu")
    except RuntimeError:
        checkpoint = torch.load(str(path), map_location="cpu", weights_only=False)
        model = checkpoint["model"] if isinstance(checkpoint, dict) and "model" in checkpoint else checkpoint

    model.eval()
    return model


def predict_with_model(model: Any, image_rgb: np.ndarray) -> tuple[str, float]:
    """Run a model that returns three logits or probabilities."""
    import torch

    resized = cv2.resize(image_rgb, (IMAGE_SIZE, IMAGE_SIZE)).astype(np.float32) / 255.0
    tensor = torch.from_numpy(resized).permute(2, 0, 1).unsqueeze(0)
    with torch.inference_mode():
        output = model(tensor)
        if isinstance(output, (tuple, list)):
            output = output[0]
        scores = torch.softmax(output, dim=1)[0]
        class_index = int(torch.argmax(scores).item())
        confidence = float(scores[class_index].item())
    return STAGES[class_index], confidence


def heuristic_prediction(image_bgr: np.ndarray) -> tuple[str, float]:
    """Use green coverage and contour area as a useful no-model demo."""
    hsv = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2HSV)
    green_mask = cv2.inRange(hsv, np.array([25, 35, 25]), np.array([95, 255, 255]))
    green_ratio = float(np.count_nonzero(green_mask)) / float(green_mask.size)

    contours, _ = cv2.findContours(green_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    largest_ratio = 0.0
    if contours:
        largest_area = max(cv2.contourArea(contour) for contour in contours)
        largest_ratio = largest_area / float(green_mask.shape[0] * green_mask.shape[1])

    if green_ratio < 0.08 or largest_ratio < 0.015:
        stage = "seedling"
    elif green_ratio < 0.28 or largest_ratio < 0.16:
        stage = "vegetative"
    else:
        stage = "mature"

    # This is a confidence-like indicator, not a calibrated probability.
    confidence = min(0.88, 0.55 + abs(green_ratio - 0.18))
    return stage, confidence


def main() -> None:
    args = parse_args()
    image_bgr, image_rgb = load_image(args.image)
    model = load_torch_model(args.model)

    if model is None:
        stage, confidence = heuristic_prediction(image_bgr)
        source = "opencv-heuristic"
        warning = "No trained checkpoint found; using the starter OpenCV heuristic."
    else:
        stage, confidence = predict_with_model(model, image_rgb)
        source = str(args.model)
        warning = None

    result = {
        "image": str(args.image),
        "stage": stage,
        "confidence": round(confidence, 4),
        "source": source,
    }
    if warning:
        result["warning"] = warning

    if args.json:
        print(json.dumps(result, indent=2))
    else:
        print(f"Predicted stage: {stage} ({confidence:.1%} confidence)")
        print(f"Source: {source}")
        if warning:
            print(f"Note: {warning}")


if __name__ == "__main__":
    main()
