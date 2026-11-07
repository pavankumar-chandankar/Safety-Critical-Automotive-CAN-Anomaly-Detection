import logging
from data_acquisition import load_public_transport_data
from data_preprocessing import preprocess_data
from model_training import train_prophet_model, save_model
from anomaly_detection import load_model, make_predictions
from evaluation import evaluate_predictions

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")

if __name__ == "__main__":
    logging.info("Starting time series forecasting workflow...")

    # 1. Data Acquisition
    transport_data = load_public_transport_data()
    if transport_data is None:
        logging.error("Data acquisition failed. Exiting.")
        exit(1)

    # 2. Data Preprocessing
    target_column = "Count"  # Replace with the actual target column name
    train_data = transport_data.iloc[:-24]  # Use all but the last 24 hours for training
    test_data = transport_data.iloc[-24:]  # Use the last 24 hours for testing
    preprocessed_train_data = preprocess_data(train_data.copy(), target_column)
    preprocessed_test_data = preprocess_data(test_data.copy(), target_column)

    # 3. Model Training
    model = train_prophet_model(preprocessed_train_data, target_column)
    save_model(model)

    # 4. Prediction
    predictions = make_predictions(model, preprocessed_test_data, target_column)

    # 5. Evaluation
    evaluate_predictions(predictions, test_data, target_column)

    logging.info("Time series forecasting workflow completed successfully.")
