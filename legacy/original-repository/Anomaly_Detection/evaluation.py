import logging
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from sklearn.metrics import (
    mean_absolute_error,
    mean_squared_error,
    r2_score,
    mean_absolute_percentage_error,
    explained_variance_score,
    median_absolute_error,
    mean_squared_log_error,
)
from statsmodels.graphics.tsaplots import plot_acf, plot_pacf
from statsmodels.tsa.stattools import adfuller, kpss
from scipy.stats import shapiro, probplot, boxcox
from anomaly_detection import load_model, make_predictions
from data_preprocessing import preprocess_data
from data_acquisition import load_public_transport_data

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")


def evaluate_predictions(predictions, actual, target_column):
    """
    Evaluates the predictions using various metrics, visualizations, and statistical tests.

    Args:
        predictions (pd.DataFrame): The DataFrame with predictions.
        actual (pd.DataFrame): The DataFrame with actual values.
        target_column (str): The name of the target column.
    """
    logging.info("Evaluating predictions...")

    # Calculate metrics
    mae = mean_absolute_error(actual[target_column], predictions["Prediction"])
    rmse = mean_squared_error(actual[target_column], predictions["Prediction"], squared=False)
    r2 = r2_score(actual[target_column], predictions["Prediction"])
    mape = mean_absolute_percentage_error(actual[target_column], predictions["Prediction"])
    explained_variance = explained_variance_score(
        actual[target_column], predictions["Prediction"]
    )
    medae = median_absolute_error(actual[target_column], predictions["Prediction"])
    try:
        msle = mean_squared_log_error(actual[target_column], predictions["Prediction"])
    except ValueError:
        logging.warning("MSLE calculation failed due to negative or zero values.")
        msle = None

    logging.info(f"Mean Absolute Error (MAE): {mae}")
    logging.info(f"Root Mean Squared Error (RMSE): {rmse}")
    logging.info(f"R-squared (R2): {r2}")
    logging.info(f"Mean Absolute Percentage Error (MAPE): {mape}")
    logging.info(f"Explained Variance Score: {explained_variance}")
    logging.info(f"Median Absolute Error (MedAE): {medae}")
    if msle is not None:
        logging.info(f"Mean Squared Log Error (MSLE): {msle}")

    # Create visualizations
    plt.figure(figsize=(12, 6))
    plt.plot(actual[target_column], label="Actual")
    plt.plot(predictions["Prediction"], label="Prediction")
    plt.legend()
    plt.title("Actual vs. Predicted Values")
    plt.show()

    # Plot residuals
    residuals = actual[target_column] - predictions["Prediction"]
    plt.figure(figsize=(12, 6))
    plt.plot(residuals)
    plt.title("Residuals")
    plt.show()

    # Histogram of residuals
    plt.figure(figsize=(12, 6))
    plt.hist(residuals, bins=20)
    plt.title("Histogram of Residuals")
    plt.show()

    # ACF and PACF plots of residuals
    fig, axes = plt.subplots(1, 2, figsize=(12, 4))
    plot_acf(residuals, ax=axes)
    plot_pacf(residuals, ax=axes)
    plt.tight_layout()
    plt.show()

    # Q-Q plot of residuals
    plt.figure(figsize=(12, 6))
    probplot(residuals, dist="norm", plot=plt)
    plt.title("Q-Q Plot of Residuals")
    plt.show()

    # Shapiro-Wilk test for normality of residuals
    shapiro_test_stat, shapiro_p_value = shapiro(residuals)
    logging.info(f"Shapiro-Wilk Test for Normality (Residuals):")
    logging.info(f"  Test Statistic: {shapiro_test_stat}")
    logging.info(f"  P-value: {shapiro_p_value}")

    # Box-Cox transformation of target variable
    try:
        transformed_actual, lambda_value = boxcox(actual[target_column])
        logging.info(f"Box-Cox Transformation (lambda = {lambda_value}):")
        plt.figure(figsize=(12, 6))
        plt.hist(transformed_actual, bins=20)
        plt.title("Histogram of Transformed Actual Values")
        plt.show()
    except ValueError:
        logging.warning("Box-Cox transformation failed due to non-positive values.")

    # Stationarity tests (ADF and KPSS)
    adf_result = adfuller(actual[target_column])
    kpss_result = kpss(actual[target_column], regression="c", nlags="auto")
    logging.info("Stationarity Tests:")
    logging.info("  Augmented Dickey-Fuller (ADF) Test:")
    logging.info(f"    Test Statistic: {adf_result}")
    logging.info(f"    P-value: {adf_result}")
    logging.info(f"    Critical Values: {adf_result}")
    logging.info("  Kwiatkowski-Phillips-Schmidt-Shin (KPSS) Test:")
    logging.info(f"    Test Statistic: {kpss_result}")
    logging.info(f"    P-value: {kpss_result}")
    logging.info(f"    Critical Values: {kpss_result}")

    logging.info("Evaluation completed.")


if __name__ == "__main__":
    transport_data = load_public_transport_data()
    if transport_data is not None:
        target_column = "Count"  # Replace with the actual target column name
        train_data = transport_data.iloc[
            :-24
        ]  # Use all but the last 24 hours for training
        test_data = transport_data.iloc[-24:]  # Use the last 24 hours for testing
        preprocessed_train_data = preprocess_data(train_data.copy(), target_column)
        preprocessed_test_data = preprocess_data(test_data.copy(), target_column)
        model = load_model()
        predictions = make_predictions(model, preprocessed_test_data, target_column)
        evaluate_predictions(predictions, test_data, target_column)
    else:
        logging.error("Evaluation failed due to data loading error.")
